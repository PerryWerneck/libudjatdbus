/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

 /**
  * @brief Implements abstract connection.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <dbus/dbus.h>
 #include <string>
 #include <mutex>
 #include <udjat/tools/dbus/connection.h>
 #include <udjat/tools/dbus/interface.h>
 #include <udjat/tools/dbus/message.h>
 #include <udjat/tools/dbus/signal.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/mainloop.h>
 #include <private/mainloop.h>
 #include <udjat/tools/string.h>

 using namespace std;

 namespace Udjat {

	class DataSlot {
	private:
		dbus_int32_t slot = -1; // The passed-in slot must be initialized to -1, and is filled in with the slot ID
		DataSlot() {
			dbus_connection_allocate_data_slot(&slot);
			Logger::String{"Got slot '",slot,"' for connection watchdog"}.trace("d-bus");
		}

	public:

		~DataSlot() {
			dbus_connection_free_data_slot(&slot);
		}

		static DataSlot & getInstance() {
			static DataSlot instance;
			return instance;
		}

		inline dbus_int32_t value() const noexcept {
			return slot;
		}

	};

	std::mutex Abstract::DBus::Connection::guard;

	static void trace_connection_free(const Abstract::DBus::Connection *connection) {
		Logger::String("Connection '",((unsigned long) connection),"' was released").trace("d-bus");
	}

	DBusConnection * Abstract::DBus::Connection::SharedConnectionFactory(DBusBusType type) {

		DBusError err;
		dbus_error_init(&err);

		DBusConnection * connct = dbus_bus_get(type, &err);
		if(dbus_error_is_set(&err)) {
			std::string message(err.message);
			dbus_error_free(&err);
			throw std::runtime_error(message);
		}

		return connct;

	}

	Abstract::DBus::Connection::Connection(const char *name, DBusConnection *c) : object_name{name}, conn{c} {

		static bool initialized = false;
		if(!initialized) {

			initialized = true;

			// Initialize d-bus threads.
			Logger::String("Initializing d-bus thread system").trace("d-bus");
			dbus_threads_init_default();

		}

	}

	Abstract::DBus::Connection::~Connection() {
	}

	void Abstract::DBus::Connection::open() {

		lock_guard<mutex> lock(guard);

		// Keep running if d-bus disconnect.
		dbus_connection_set_exit_on_disconnect(conn, false);

		// Add message filter.
		if (dbus_connection_add_filter(conn, (DBusHandleMessageFunction) on_message, this, NULL) == FALSE) {
			throw std::runtime_error("Cant add filter to D-Bus connection");
		}

		// Initialize Main loop.
		MainLoop::getInstance();

		// Set watch functions.
		if(!dbus_connection_set_watch_functions(
			conn,
			(DBusAddWatchFunction) add_watch,
			(DBusRemoveWatchFunction) remove_watch,
			(DBusWatchToggledFunction) toggle_watch,
			this,
			nullptr)
		) {
			throw runtime_error("dbus_connection_set_watch_functions has failed");
		}

		// Set timeout functions.
		if(!dbus_connection_set_timeout_functions(
			conn,
			(DBusAddTimeoutFunction) add_timeout,
			(DBusRemoveTimeoutFunction) remove_timeout,
			(DBusTimeoutToggledFunction) toggle_timeout,
			this,
			nullptr)
		) {
			throw runtime_error("dbus_connection_set_timeout_functions has failed");
		}


		if(Logger::enabled(Logger::Trace)) {

			dbus_connection_set_data(conn,DataSlot::getInstance().value(),this,(DBusFreeFunction) trace_connection_free);

			int fd = -1;
			if(dbus_connection_get_socket(conn,&fd)) {
				Logger::String("Allocating connection '",((unsigned long) this),"' with socket '",fd,"'").trace(name());
			} else {
				Logger::String("Allocating connection '",((unsigned long) this),"'").trace(name());
			}

		}

	}

	void Abstract::DBus::Connection::bus_register() {

		DBusError err;
		dbus_error_init(&err);

		dbus_bus_register(conn,&err);
		if(dbus_error_is_set(&err)) {
			std::string message(err.message);
			dbus_error_free(&err);
			throw std::runtime_error(message);
		}

	}

	void Abstract::DBus::Connection::close() {

		lock_guard<mutex> lock(guard);

        if(Logger::enabled(Logger::Trace)) {
			int fd = -1;
			if(dbus_connection_get_socket(conn,&fd)) {
				Logger::String("Dealocating connection '",((unsigned long) this),"' from socket '",fd,"'").trace(name());
			} else {
				Logger::String("Dealocating connection '",((unsigned long) this),"'").trace(name());
			}
        }

		flush();

		// Remove interfaces.
		interfaces.remove_if([this](Udjat::DBus::Interface &intf) {
			remove(intf);
			return true;
		});

		// Remove filter
		dbus_connection_remove_filter(conn,(DBusHandleMessageFunction) on_message, this);

		Logger::String{"Restoring d-bus watchers"}.trace(name());

		if(!dbus_connection_set_watch_functions(
			conn,
			(DBusAddWatchFunction) NULL,
			(DBusRemoveWatchFunction) NULL,
			(DBusWatchToggledFunction) NULL,
			this,
			nullptr)
		) {
			Logger::String{"dbus_connection_set_watch_functions failed"}.error(name());
		}

		if(!dbus_connection_set_timeout_functions(
			conn,
			(DBusAddTimeoutFunction) NULL,
			(DBusRemoveTimeoutFunction) NULL,
			(DBusTimeoutToggledFunction) NULL,
			NULL,
			nullptr)
		) {
			Logger::String{"dbus_connection_set_timeout_functions failed"}.error(name());
		}

	}

	DBusHandlerResult Abstract::DBus::Connection::on_message(DBusConnection *, DBusMessage *message, Abstract::DBus::Connection *connection) noexcept {
		return connection->filter(message);
	}

	DBusHandlerResult Abstract::DBus::Connection::filter(DBusMessage *message) noexcept {

		lock_guard<mutex> lock(guard);

		int type = dbus_message_get_type(message);
		const char *interface = dbus_message_get_interface(message);
		const char *member = dbus_message_get_member(message);

		if(Logger::enabled(Logger::Trace)) {
			Logger::String{"Signal ", interface," ",member}.trace(name());
		}

		for(const auto &intf : interfaces) {

			if(intf == interface) {

				for(const auto &imemb : intf) {

					if(imemb == type && imemb == member) {

						try {

							debug("Processing ",interface,".",member);
							Udjat::DBus::Message msg(message);
							imemb.call(msg);

						} catch(const std::exception &e) {

							Logger::String{interface,".",member,": ",e.what()}.error(name());

						} catch(...) {

							Logger::String{interface,".",member,": Unexpecter error"}.error(name());

						}

					}

				}

			}

		}

		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

	}

	void Abstract::DBus::Connection::flush() noexcept {
		dbus_connection_flush(conn);
	}

	void Abstract::DBus::Connection::insert(const Udjat::DBus::Interface &interface) {

		Logger::String{"Connecting to '",interface.rule().c_str(),"'"}.trace(name());

		DBusError error;
		dbus_error_init(&error);

		dbus_bus_add_match(conn,interface.rule().c_str(), &error);
		dbus_connection_flush(conn);

		if (dbus_error_is_set(&error)) {
			Logger::String message{"Error '",error.message,"' adding interface"};
			dbus_error_free(&error);
			throw std::runtime_error(message);
		}

	}

	void Abstract::DBus::Connection::remove(const Udjat::DBus::Interface &interface) {

		Logger::String{"Disconnecting from '",interface.rule().c_str(),"'"}.trace(name());

		DBusError error;
		dbus_error_init(&error);

		dbus_bus_remove_match(conn,interface.rule().c_str(), &error);

		if(dbus_error_is_set(&error)) {
			Logger::String{"Error '",error.message,"' removing interface '",interface.c_str(),"'"}.error(name());
			dbus_error_free(&error);
		}

	}

	void Abstract::DBus::Connection::push_back(Udjat::DBus::Interface &intf) {
		lock_guard<mutex> lock(guard);
		insert(intf);
		interfaces.push_back(intf);
	}

	Udjat::DBus::Interface & Abstract::DBus::Connection::emplace_back(const char *intf) {

		if(!(intf && *intf)) {
			throw system_error(EINVAL,system_category(),"A dbus interface name is required");
		}

		lock_guard<mutex> lock(guard);

		for(auto &inserted : interfaces) {
			if(!strcasecmp(inserted.c_str(),intf)) {
				Logger::String{"Already watching '",intf,"'"}.trace(name());
				return inserted;
			}
		}

		Udjat::DBus::Interface & interface = interfaces.emplace_back(intf);
		insert(interface);

		return interface;
	}

	void Abstract::DBus::Connection::push_back(const XML::Node &node) {
		Udjat::DBus::Interface intf{node};
		return push_back(intf);
	}

	Udjat::DBus::Member & Abstract::DBus::Connection::subscribe(const char *interface, const char *member, const std::function<bool(Udjat::DBus::Message &message)> &callback) {
		return emplace_back(interface).emplace_back(member,callback);
	}

	void Abstract::DBus::Connection::remove(const Udjat::DBus::Member &member) {

		lock_guard<mutex> lock(guard);
		interfaces.remove_if([this,&member](Udjat::DBus::Interface &interface){

			interface.remove(member);

			if(interface.empty()) {
				remove(interface);
				return true;
			}

			return false;

		});

	}

	void Abstract::DBus::Connection::signal(const Udjat::DBus::Signal &sig) {

		lock_guard<mutex> lock(guard);

		dbus_bool_t rc = dbus_connection_send(conn, sig.dbus_message(), NULL);
		dbus_connection_flush(conn);

		if(!rc) {
			throw runtime_error("Can't send D-Bus signal");
		}

	}

 }

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
 #include <udjat/version.h>
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
 #include <private/mainloop.h>

 using namespace std;

 namespace Udjat {

	std::mutex Abstract::DBus::Connection::guard;

	void DBus::initialize() {
		static bool initialized = false;
		if(!initialized) {
			initialized = true;
			Logger::String("Initializing d-bus thread system").trace("d-bus");
			dbus_threads_init_default();

		}
	}

	DBusConnection * Abstract::DBus::Connection::ConnectionFactory(const XML::Node &node) {

#if UDJAT_CHECK_VERSION(1,2,0)
		Udjat::String bus{node, "dbus-bus-name", "starter"};
#else
		std::string bus = Udjat::XML::StringFactory(node, "dbus-bus-name", "system", "starter");
#endif // UDJAT_CHECK_VERSION

		if(!strcasecmp(bus.c_str(),"system")) {
			return Udjat::DBus::SystemBus::ConnectionFactory();
		}

		if(!strcasecmp(bus.c_str(),"session")) {
			return Udjat::DBus::SessionBus::ConnectionFactory();
		}

		if(!strcasecmp(bus.c_str(),"starter")) {
			return Udjat::DBus::StarterBus::ConnectionFactory();
		}

		throw runtime_error(Logger::String{"Unexpected bus name: '",bus,"'"});

	}

	std::shared_ptr<Abstract::DBus::Connection> Abstract::DBus::Connection::factory(const XML::Node &node) {

#if UDJAT_CHECK_VERSION(1,2,0)
		Udjat::String bus{node, "dbus-bus-name", "starter"};
#else
		std::string bus = Udjat::XML::StringFactory(node, "dbus-bus-name", "system", "starter");
#endif // UDJAT_CHECK_VERSION

		if(!strcasecmp(bus.c_str(),"system")) {
			return make_shared<Udjat::DBus::SystemBus>();
		}

		if(!strcasecmp(bus.c_str(),"session")) {
			return make_shared<Udjat::DBus::SessionBus>();
		}

		if(!strcasecmp(bus.c_str(),"starter")) {
			return make_shared<Udjat::DBus::StarterBus>();
		}

		throw runtime_error(Logger::String{"Unexpected bus name: '",bus,"'"});
	}

	Abstract::DBus::Connection::Connection(const char *name, DBusConnection *c) : object_name{name}, conn{c} {

		// Keep running if d-bus disconnect.
		dbus_connection_set_exit_on_disconnect(conn, false);

		lock_guard<mutex> lock(guard);

		try {

			// Add message filter.
			if (dbus_connection_add_filter(conn, (DBusHandleMessageFunction) on_message, this, NULL) == FALSE) {
				throw std::runtime_error("Cant add filter to D-Bus connection");
			}

			if(Logger::enabled(Logger::Debug)) {

				int fd = -1;
				if(dbus_connection_get_socket(conn,&fd)) {
					Logger::String("Allocating connection '",((unsigned long) this),"' with socket '",fd,"'").write(Logger::Debug,name);
				} else {
					Logger::String("Allocating connection '",((unsigned long) this),"'").write(Logger::Debug,name);
				}

			}

		} catch(...) {

			if(conn) {
				Logger::String{"Closing private connection due to initialization error"}.error(name);
				dbus_connection_unref(conn);
				conn = NULL;
			}

			throw;

		}

	}

	void Abstract::DBus::Connection::clear() {

		lock_guard<mutex> lock(guard);

		flush();

		// Remove interfaces.
		interfaces.remove_if([this](Udjat::DBus::Interface &intf) {
			remove(intf);
			return true;
		});

		// Remove filter
		dbus_connection_remove_filter(conn,(DBusHandleMessageFunction) on_message, this);


	}

	Abstract::DBus::Connection::~Connection() {
		// Release connection
		dbus_connection_unref(conn);
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

	DBusHandlerResult Abstract::DBus::Connection::on_message(DBusConnection *, DBusMessage *message, Abstract::DBus::Connection *connection) noexcept {

		lock_guard<mutex> lock(connection->guard);

		try {

			return connection->filter(message);

		} catch(const std::exception &e) {

			Logger::String{
				dbus_message_get_interface(message),
				".",
				dbus_message_get_member(message),
				": ",
				e.what()
			}.error(connection->name());

		} catch(...) {

			Logger::String{
				dbus_message_get_interface(message),
				".",
				dbus_message_get_member(message),
				": Unexpected error",
			}.error(connection->name());

		}

		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

	}


	DBusHandlerResult Abstract::DBus::Connection::filter(DBusMessage *message) {

//		int type = dbus_message_get_type(message);
//		const char *member = dbus_message_get_member(message);

//		if(Logger::enabled(Logger::Trace)) {
//			Logger::String{type_name(type)," ",interface," ",member," ",dbus_message_get_path(message)}.trace(name());
//		}

		const char *interface = dbus_message_get_interface(message);
		for(const auto &intf : interfaces) {

			if(intf == interface) {

				DBusHandlerResult rc = intf.filter(message);
				if(rc != DBUS_HANDLER_RESULT_NOT_YET_HANDLED) {
					return rc;
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
			Logger::String message{"Error '",error.message,"' adding rule ",interface.rule().c_str()};
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

	void Abstract::DBus::Connection::remove(Udjat::DBus::Interface &intf) {
		lock_guard<mutex> lock(guard);
		interfaces.remove(intf);
	}

	Udjat::DBus::Interface & Abstract::DBus::Connection::emplace_back(const char *intf) {

		if(!(intf && *intf)) {
			throw system_error(EINVAL,system_category(),"A dbus interface name is required");
		}

		lock_guard<mutex> lock(guard);

		for(auto &inserted : interfaces) {
			if(!strcasecmp(inserted.c_str(),intf)) {
				Logger::String{"Already watching '",intf,"'"}.write(Logger::Debug,name());
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

	int Abstract::DBus::Connection::request_name(const char *name) {

		DBusError err;
		dbus_error_init(&err);

		int reqstatus = dbus_bus_request_name(
			conn,
			name,
			DBUS_NAME_FLAG_REPLACE_EXISTING,
			&err
		);

		if(dbus_error_is_set(&err)) {
			std::string message{err.message};
			dbus_error_free(&err);
			throw std::runtime_error(message);
		}

		return reqstatus;

	}

 }

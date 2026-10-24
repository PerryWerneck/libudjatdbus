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
 #include <memory>
 
 #include <udjat/tools/dbus/connection.h>
 #include <udjat/tools/dbus/message.h>
 #include <udjat/tools/dbus/signal.h>
 #include <udjat/tools/dbus/exception.h>
 #include <udjat/tools/dbus/message.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/string.h>
 #include <mutex>
 
 #include <private/mainloop.h>
 
 using namespace std;

 namespace Udjat {

	std::mutex DBus::Connection::guard;

	bool DBus::initialize() {
		static bool initialized = false;
		if(!initialized) {
			initialized = true;
			Logger::String("Initializing d-bus thread system").trace();
			dbus_threads_init_default();
			return true;
		}
		return false;
	}

	DBusBusType UDJAT_API DBus::BusTypeFactory(const Udjat::Properties &props) {

		String type{props["dbus-bus-name"]};
		if(type.empty()) {
			type = String{props["bus-name"]};
		}

		if(type.empty()) {
			Logger::String{"Required attribute bus-name is missing on '",props.path().c_str(),"', using default"}.warning();
			return DBUS_BUS_STARTER;
		}

		static const struct {
			DBusBusType type;
			const char *name;
		} busnames[] = {
			{ DBUS_BUS_SYSTEM, "system"		},
			{ DBUS_BUS_SESSION, "session"	},
			{ DBUS_BUS_STARTER, "starter"	},
		};

		for(const auto &bus : busnames) {
			if(!strcasecmp(type.c_str(),bus.name)) {
				return bus.type;
			}
		}

		throw runtime_error("Required attribute bus-name is invalid");

	}

	DBus::Connection & DBus::Connection::getInstance(const Udjat::Properties &props) {
		return getInstance(DBus::BusTypeFactory(props));
	}

	DBus::Connection & DBus::Connection::getInstance(DBusBusType bustype) {

		switch(bustype) {
		case DBUS_BUS_SYSTEM:
			return DBus::SystemBus::getInstance();

		case DBUS_BUS_SESSION:
			return DBus::SessionBus::getInstance();

		case DBUS_BUS_STARTER:
			return DBus::StarterBus::getInstance();
		}

		throw system_error(EINVAL,system_category(),"Invalid bus type");

	}

	DBus::Connection::Connection(const char *name, DBusConnection *c) : conn{c}, object_name{name} {

		lock_guard<mutex> lock(guard);

		initialize();

		// Keep running if d-bus disconnect.
		dbus_connection_set_exit_on_disconnect(conn, false);

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

	void DBus::Connection::clear() {

		lock_guard<mutex> lock(guard);

		flush();

		// Remove filter
		dbus_connection_remove_filter(conn,(DBusHandleMessageFunction) on_message, this);

	}

	DBus::Connection::~Connection() {
		// Release connection
		dbus_connection_unref(conn);
	}

	void DBus::Connection::bus_register() {

		DBus::Error err;
		dbus_bus_register(conn,err);
		err.verify();

	}

	DBusHandlerResult DBus::Connection::on_message(DBusConnection *, DBusMessage *message, DBus::Connection *connection) noexcept {

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


	DBusHandlerResult DBus::Connection::filter(DBusMessage *message) {

		const char *interface = dbus_message_get_interface(message);
		int type = dbus_message_get_type(message);
		DBusHandlerResult rc = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

		debug("----------------> Check for interface '",interface,"'");

		for(const auto &intf : interfaces) {

			if(intf != interface) {
				continue;
			}

			debug("Found interface '",intf.c_str(),"' checking ",intf.subscriptions.size()," subscriptions");

			const char *member = dbus_message_get_member(message);

			for(const auto &subscription : intf.subscriptions) {

				if(subscription->match(member,type)) {

					try {

						DBus::Message msg{message};

						if(subscription->process(msg)) {
							rc = DBUS_HANDLER_RESULT_HANDLED;
							if(type != DBUS_MESSAGE_TYPE_SIGNAL) {
								return rc;
							}
						}

					} catch(const std::exception &e) {

						Logger::String{intf.c_str(),".",subscription->c_str(),": ",e.what()}.error();

					} catch(...) {

						Logger::String{intf.c_str(),".",subscription->c_str(),": Unexpected error"}.error();

					}

				}

			}

		}

		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

	}

	bool DBus::Connection::name_has_owner(const char *name) const {

		DBus::Error error;
		dbus_bool_t rc = dbus_bus_name_has_owner(conn, name, error);
		error.verify();
		return rc;

	};

	void DBus::Connection::flush() noexcept {
		dbus_connection_flush(conn);
	}

	DBus::Connection::Interface & DBus::Connection::InterfaceFactory(const char *name) {

		for(Interface &intf : interfaces) {

			if(!strcmp(name,intf.c_str())) {
				return intf;
			}

		}

#if __cplusplus >= 201703
		Interface &intf = interfaces.emplace_back(name);
#else
		interfaces.emplace_back(name);
		Interface &intf = interfaces.back();
#endif

		// Add interface
		Logger::String{"Connecting to '",name,"'"}.trace(this->name());

		DBusError error;
		dbus_error_init(&error);

		auto rule = intf.rule("signal");
		debug("RULE='",rule.c_str(),"'");

		dbus_bus_add_match(conn,rule.c_str(), &error);
		dbus_connection_flush(conn);

		if (dbus_error_is_set(&error)) {
			Logger::String message{"Error '",error.message,"' adding rule ",intf.rule("signal").c_str()};
			dbus_error_free(&error);
			throw std::runtime_error(message);
		}

		return intf;
	}

	std::shared_ptr<DBus::Connection::Subscription> DBus::Connection::subscribe(const char *interface, const char *member, const std::function<bool(Message &message)> &callback) {

		class Subscription : public DBus::Connection::Subscription {
		public:
			const std::function<bool(Message &message)> &callback;

			Subscription(const char *name, const std::function<bool(DBus::Message &message)> &c) : DBus::Connection::Subscription{name}, callback{c} {
			}

			bool process(Message &message) override {
				return callback(message);
				return false;
			}
		};

		auto subscription = make_shared<Subscription>(member,callback);
		subscribe(interface,subscription);
		return subscription;
	}

	void DBus::Connection::subscribe(const char *interface, std::shared_ptr<DBus::Connection::Subscription> subscription) {
		Logger::String{"Subscribing ",interface,".",subscription->c_str()}.trace(this->name());
		lock_guard<mutex> lock(guard);
		InterfaceFactory(interface).subscriptions.push_back(subscription);
	}


	bool DBus::Connection::unsubscribe(std::shared_ptr<DBus::Connection::Subscription> subscription) {

		if(!subscription) {
			return false;
		}

		lock_guard<mutex> lock(guard);

		for(auto &intf : interfaces) {

			bool rc = false;
			intf.subscriptions.remove_if([this,&intf,&rc,&subscription](std::shared_ptr<Subscription> sb){
				if(sb.get() == subscription.get()) {
					Logger::String{"Unsubscribing ",intf.c_str(),".",subscription->c_str()}.trace(this->name());
					rc = true;
					return true;
				}
				return false;
			});

			if(rc) {

				if(intf.subscriptions.empty()) {

					Logger::String{"Disconnecting from '",intf.c_str(),"'"}.trace(this->name());

					debug("RULE='",intf.rule("signal").c_str(),"'");

					DBusError error;
					dbus_error_init(&error);

					dbus_bus_remove_match(conn,intf.rule("signal").c_str(), &error);

					if(dbus_error_is_set(&error)) {
						Logger::String{"Error '",error.message,"' removing interface '",intf.c_str(),"'"}.error(name());
						dbus_error_free(&error);
					}


				}

				interfaces.remove(intf);
				
				return rc;
			}

		}

		return false;

	}

	// void DBus::Connection::insert(const Udjat::DBus::Interface &interface) {

	// 	Logger::String{"Connecting to '",interface.rule().c_str(),"'"}.trace(name());

	// 	DBusError error;
	// 	dbus_error_init(&error);

	// 	dbus_bus_add_match(conn,interface.rule().c_str(), &error);
	// 	dbus_connection_flush(conn);

	// 	if (dbus_error_is_set(&error)) {
	// 		Logger::String message{"Error '",error.message,"' adding rule ",interface.rule().c_str()};
	// 		dbus_error_free(&error);
	// 		throw std::runtime_error(message);
	// 	}

	// }

	// void DBus::Connection::remove(const Udjat::DBus::Interface &interface) {

	// 	Logger::String{"Disconnecting from '",interface.rule().c_str(),"'"}.trace(name());

	// 	DBusError error;
	// 	dbus_error_init(&error);

	// 	dbus_bus_remove_match(conn,interface.rule().c_str(), &error);

	// 	if(dbus_error_is_set(&error)) {
	// 		Logger::String{"Error '",error.message,"' removing interface '",interface.c_str(),"'"}.error(name());
	// 		dbus_error_free(&error);
	// 	}

	// }

	// void DBus::Connection::push_back(Udjat::DBus::Interface &intf) {
	// 	lock_guard<mutex> lock(guard);
	// 	insert(intf);
	// 	interfaces.push_back(intf);
	// }

// 	Udjat::DBus::Interface & DBus::Connection::emplace_back(const char *intf) {

// 		if(!(intf && *intf)) {
// 			throw system_error(EINVAL,system_category(),"A dbus interface name is required");
// 		}

// 		lock_guard<mutex> lock(guard);

// 		for(auto &inserted : interfaces) {
// 			if(!strcasecmp(inserted.c_str(),intf)) {
// 				Logger::String{"Already watching '",intf,"'"}.write(Logger::Debug,name());
// 				return inserted;
// 			}
// 		}

// #if __cplusplus >= 201703
// 		Udjat::DBus::Interface & interface = interfaces.emplace_back(intf);
// #else
// 		interfaces.emplace_back(intf);
// 		Udjat::DBus::Interface & interface = interfaces.back();
// #endif
// 		insert(interface);

// 		return interface;
// 	}

	// void DBus::Connection::push_back(const XML::Node &node) {
	// 	Udjat::DBus::Interface intf{node};
	// 	return push_back(intf);
	// }

	// Udjat::DBus::Member & DBus::Connection::subscribe(const char *interface, const char *member, const std::function<bool(Udjat::DBus::Message &message)> &callback) {
	// 	return emplace_back(interface).emplace_back(member,callback);
	// }

	// void DBus::Connection::remove(Udjat::DBus::Interface &intf) {
	// 	lock_guard<mutex> lock(guard);
	// 	interfaces.remove(intf);
	// }

	// void DBus::Connection::remove(const Udjat::DBus::Member &member) {

	// 	lock_guard<mutex> lock(guard);
	// 	interfaces.remove_if([this,&member](Udjat::DBus::Interface &interface){

	// 		interface.remove(member);	
	// 		if(interface.empty()) {
	// 			Logger::String{"Unwatching '",interface.c_str(),"'"}.trace(name());
	// 			return true;
	// 		}
	// 		return false;

	// 	});

	// }

	void DBus::Connection::signal(const Udjat::DBus::Signal &sig) {

		lock_guard<mutex> lock(guard);

		dbus_bool_t rc = dbus_connection_send(conn, sig.dbus_message(), NULL);
		dbus_connection_flush(conn);

		if(!rc) {
			throw runtime_error("Can't send D-Bus signal");
		}

	}

	int DBus::Connection::request_name(const char *name) {

		DBus::Error err;

		int reqstatus = dbus_bus_request_name(
			conn,
			name,
			DBUS_NAME_FLAG_REPLACE_EXISTING,
			err
		);

		err.verify();

		return reqstatus;

	}

 }

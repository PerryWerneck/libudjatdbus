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
  * @brief Declare D-Bus service object.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/version.h>
 #include <dbus/dbus.h>
 #include <udjat/tools/dbus/defs.h>
 #include <udjat/tools/dbus/connection.h>
 #include <udjat/tools/service.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/dbus/service.h>
 #include <udjat/tools/exception.h>
 #include <stdexcept>
 #include <udjat/tools/intl.h>

 using namespace std;

 namespace Udjat {

	DBusConnection * connection_factory(const XML::Node &node) {

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

	DBus::Service::Service(const ModuleInfo &module, DBusConnection *c, const char *name, const char *destination)
		: Udjat::Service{name,module}, conn{c}, dest{destination} {

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

	DBus::Service::Service(const ModuleInfo &module, const char *name, const char *destination)
		: Service{module,Udjat::DBus::StarterBus::ConnectionFactory(),name,destination} {
	}

	DBus::Service::Service(const ModuleInfo &module, const XML::Node &node)
		: Service{
			module,
			connection_factory(node),
			String{node,"name","d-bus"}.as_quark(),
			String{node,"dbus-name",true}.as_quark()
		} {
	}

	DBus::Service::~Service() {
		dbus_connection_unref(conn);
	}

	void DBus::Service::start() {
	}

	void DBus::Service::stop() {
	}

	DBusHandlerResult DBus::Service::on_message(DBusConnection *connct, DBusMessage *message, DBus::Service *service) noexcept {

		if(strcasecmp(service->dest,dbus_message_get_destination(message)) != 0) {
			return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
		}

		Udjat::DBus::Message request{message};

		switch(dbus_message_get_type(message)) {
		case DBUS_MESSAGE_TYPE_METHOD_CALL:
			try {

				DBus::Message response{dbus_message_new_method_return(message)};
				if(!service->on_method(request,response)) {
					return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
				}

				// Send response
				dbus_connection_send(connct, (DBusMessage *) response, NULL);

			} catch(const std::system_error &e) {

					Logger::String{
						dbus_message_get_interface(message),
						".",
						dbus_message_get_member(message),
						": ",
						e.what()
					}.error(service->name());

					const char *name = DBUS_ERROR_FAILED;

					static const struct {
						int value;
						const char *name;
					} codes[] = {
						{ ENOTSUP,	DBUS_ERROR_NOT_SUPPORTED },
						{ EPERM,	DBUS_ERROR_ACCESS_DENIED },
					};

					int value = e.code().value();
					for(auto &code : codes) {
						if(code.value == value) {
							name = code.name;
							break;
						}
					}

					DBusMessage *response = dbus_message_new_error(
						message,
						name,
						e.what()
					);

					dbus_connection_send(connct, response, NULL);
					dbus_message_unref(response);

			} catch(const std::exception &e) {

					Logger::String{
						dbus_message_get_interface(message),
						".",
						dbus_message_get_member(message),
						": ",
						e.what()
					}.error(service->name());

					DBusMessage *response = dbus_message_new_error(
						message,
						DBUS_ERROR_FAILED,
						e.what()
					);

					dbus_connection_send(connct, response, NULL);
					dbus_message_unref(response);

			} catch(...) {

				Logger::String{
					dbus_message_get_interface(message),
					".",
					dbus_message_get_member(message),
					": Unexpected error",
				}.error(service->name());

				DBusMessage *response = dbus_message_new_error(
					message,
					DBUS_ERROR_FAILED,
					"Unexpected error"
				);

				dbus_connection_send(connct, response, NULL);
				dbus_message_unref(response);

			}
			break;

		case DBUS_MESSAGE_TYPE_SIGNAL:
			try {

				if(!service->on_signal(request)) {
					return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
				}

			} catch(const std::exception &e) {

					Logger::String{
						dbus_message_get_interface(message),
						".",
						dbus_message_get_member(message),
						": ",
						e.what()
					}.error(service->name());

			} catch(...) {

				Logger::String{
					dbus_message_get_interface(message),
					".",
					dbus_message_get_member(message),
					": Unexpected error",
				}.error(service->name());

			}
			break;

		default:
			return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

		}

		return DBUS_HANDLER_RESULT_HANDLED;

	}

	bool DBus::Service::on_signal(Udjat::DBus::Message &) {
		return false;
	}

	bool DBus::Service::on_method(Udjat::DBus::Message &, Udjat::DBus::Message &) {
		throw system_error(ENOTSUP,system_category(),_("Unsupported or unknown request"));
	}

 }

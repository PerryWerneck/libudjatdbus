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
  * @brief Implement D-Bus service object.
  */

 // References:
 //
 // https://github.com/fbuihuu/samples-dbus/blob/master/dbus-server.c
 //

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/version.h>
 #include <dbus/dbus.h>
 #include <udjat/tools/dbus/defs.h>
 #include <udjat/tools/dbus/connection.h>
 #include <udjat/tools/dbus/message.h>
 #include <udjat/tools/service.h>
 #include <udjat/tools/worker.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/dbus/service.h>
 #include <udjat/tools/exception.h>
 #include <udjat/tools/dbus/exception.h>
 #include <udjat/tools/application.h>
 #include <stdexcept>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/exception.h>
 #include <sstream>
 using namespace std;

 namespace Udjat {

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

			{
				DBus::Error err;
				dbus_bus_register(conn,err);
				err.verify();
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
			Abstract::DBus::Connection::ConnectionFactory(node),
			String{node,"name","d-bus"}.as_quark(),
			String{node,"dbus-name",true}.as_quark()
		} {
	}

	DBus::Service::~Service() {
		dbus_connection_unref(conn);
	}

	void DBus::Service::start() {

		DBus::Error err;

		debug("--------------------------> START ",dest);
		dbus_bus_request_name(conn, dest, DBUS_NAME_FLAG_REPLACE_EXISTING, err);
		err.verify();


	}

	void DBus::Service::stop() {

		DBus::Error err;

		debug("--------------------------> STOP ",dest);
		dbus_bus_release_name(conn, dest, err);
		err.verify();

	}

	DBusHandlerResult DBus::Service::on_message(DBusConnection *connct, DBusMessage *message, DBus::Service *service) noexcept {

		if(!dbus_message_has_destination(message,service->dest)) {
			return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

		} else if(dbus_message_is_method_call(message, DBUS_INTERFACE_INTROSPECTABLE, "Introspect")) {

			// TODO: Implement introspection.
			Logger::String{"Introspection wasnt implemented"}.write(Logger::Debug,"dbus");

		}  else if (dbus_message_is_method_call(message, DBUS_INTERFACE_PROPERTIES, "Get")) {

			// TODO: Implement introspection.
			Logger::String{"Introspection wasnt implemented"}.write(Logger::Debug,"dbus");

		}  else if (dbus_message_is_method_call(message, DBUS_INTERFACE_PROPERTIES, "GetAll")) {

			// TODO: Implement introspection.
			Logger::String{"Introspection wasnt implemented"}.write(Logger::Debug,"dbus");

		} else if(!strncasecmp(dbus_message_get_interface(message),PRODUCT_ID,strlen(PRODUCT_ID))) {

			switch(dbus_message_get_type(message)) {
			case DBUS_MESSAGE_TYPE_METHOD_CALL:
				try {

					DBus::Value response;
					Udjat::DBus::Message request{message};

					if(!service->on_method(request, response)) {
						return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

					}
					debug("Got response");

					DBusMessage *reply;
					if(response.empty()) {

						// Empty response - Send error.
						reply = dbus_message_new_error(
							message,
							DBUS_ERROR_FAILED,
							"Empty response from backend"
						);

					} else {

						// Load response.
						DBusMessageIter args;
						reply = dbus_message_new_method_return(message);

						dbus_message_iter_init_append(reply, &args);
						response.get(&args);
					}



					dbus_connection_send(connct, reply, NULL);
					dbus_message_unref(reply);

				} catch(const Udjat::DBus::Exception &e) {

						Logger::String{
							dbus_message_get_interface(message),
							".",
							dbus_message_get_member(message),
							": ",
							e.what()
						}.error(service->name());

						e.send(connct);

				} catch(const std::system_error &e) {

						Logger::String{
							dbus_message_get_interface(message),
							".",
							dbus_message_get_member(message),
							": ",
							e.what()
						}.error(service->name());

						DBusMessage *response;

						switch(e.code().value()) {
						case ENOTSUP:
							response = dbus_message_new_error(
								message,
								DBUS_ERROR_NOT_SUPPORTED,
								String{
									dbus_message_get_interface(message),
									".",
									dbus_message_get_member(message),
								}.c_str()
							);
							break;

						case EPERM:
							response = dbus_message_new_error(
								message,
								DBUS_ERROR_ACCESS_DENIED,
								String{
									dbus_message_get_interface(message),
									".",
									dbus_message_get_member(message),
								}.c_str()
							);
							break;

						default:
							response = dbus_message_new_error(
								message,
								DBUS_ERROR_FAILED,
								e.what()
							);
						}

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

				return DBUS_HANDLER_RESULT_HANDLED;

			case DBUS_MESSAGE_TYPE_SIGNAL:
				try {
					Udjat::DBus::Message request{message};
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
				return DBUS_HANDLER_RESULT_HANDLED;

			default:
				Logger::String{"Ignoring unknown message ",dbus_message_get_interface(message),".",dbus_message_get_member(message)}.warning("dbus");
			}


		}

		//debug("Returning DBUS_HANDLER_RESULT_NOT_YET_HANDLED for ",dbus_message_get_interface(message)," ",dbus_message_get_member(message));
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

	}

	bool DBus::Service::on_signal(Udjat::DBus::Message &) {
		return false;
	}

	bool DBus::Service::on_method(Udjat::DBus::Message &request, Udjat::Value &response) {

		// Check for standard interface
		const char *interface = dbus_message_get_interface(request);

		size_t szintf = strlen(PRODUCT_ID);
		if(strncasecmp(interface,PRODUCT_ID,szintf)) {
			debug("Ignoring request. Interface: '",interface,"' member: '",dbus_message_get_member(request),"'");
			return false;
		}

		interface += szintf;
		if(interface[0] != '.') {
			throw DBus::Exception(request,DBUS_ERROR_UNKNOWN_INTERFACE,dbus_message_get_interface(request));
		}
		interface++;

		debug("Request: '",interface,"' member: '",dbus_message_get_member(request),"'");

		if(!Worker::for_each([interface,&request,&response](const Worker &worker){

			if(!strcasecmp(worker.c_str(),interface)) {

				debug("Achei worker '",worker.c_str(),"'");

				switch(worker.call(dbus_message_get_member(request),dbus_message_get_path(request),response)) {
				case 0: // Success.
					break;

				case -1: // Failed
					throw DBus::Exception(request,DBUS_ERROR_FAILED,dbus_message_get_path(request));

				case ENOSYS:
				case ENOTSUP:
					throw DBus::Exception(request,DBUS_ERROR_NOT_SUPPORTED,dbus_message_get_member(request));

				case ENOENT: // Path not found.
					throw DBus::Exception(request,DBUS_ERROR_UNKNOWN_OBJECT,dbus_message_get_path(request));

				case ESRCH: // Method not found.
					throw DBus::Exception(request,DBUS_ERROR_UNKNOWN_METHOD,dbus_message_get_member(request));

				case EPERM: // Access to method was denied.
					throw DBus::Exception(request,DBUS_ERROR_ACCESS_DENIED,dbus_message_get_path(request));

				default:
					throw DBus::Exception(request,DBUS_ERROR_FAILED,dbus_message_get_path(request));
				}

				return true;
			}

			debug("Ignorando worker '",worker.c_str(),"'");

			return false;

		})) {
			throw DBus::Exception(request,DBUS_ERROR_UNKNOWN_INTERFACE,dbus_message_get_interface(request));
		}

		return true;

	}

 }

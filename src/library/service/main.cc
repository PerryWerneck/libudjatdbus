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
 #include <stdexcept>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/exception.h>
 #include <udjat/tools/value.h>

 #include <udjat/tools/service.h>
 #include <udjat/tools/worker.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/exception.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/interface.h>
 #include <udjat/tools/timestamp.h>

 #include <udjat/tools/dbus/defs.h>
 #include <udjat/tools/dbus/connection.h>
 #include <udjat/tools/dbus/message.h>
 #include <udjat/tools/dbus/service.h>
 #include <udjat/tools/dbus/exception.h>

 #include <sstream>

 using namespace std;

 namespace Udjat {

	DBus::Service::Service(const ModuleInfo &module, DBusConnection *c, const char *name, const char *destination)
		: Udjat::Service{name,module}, Udjat::Interface::Factory{name}, conn{c}, dest{destination} {

		// Keep running if d-bus disconnect.
		dbus_connection_set_exit_on_disconnect(conn, false);
		dbus_connection_ref(conn);

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

	DBus::Service::~Service() {
		dbus_connection_unref(conn);
	}

	DBus::Service::Service(const ModuleInfo &module, const char *name, const char *destination)
		: Service{module,Udjat::DBus::StarterBus::ConnectionFactory(),name,destination} {
	}

	/// @brief Scan XML definition for interface name.
	const char * DBus::Service::ServiceNameFactory(const XML::Node &node) {

		Application::Name appname;

		for(const char *attrname : { "dbus-service-name", "dbus-name", "service-name", "name" }) {

			String name{node,attrname};
			if(name.empty() || !strcasecmp(name.c_str(),"dbus")) {
				continue;
			}

			if(name[0] == '.') {
				name = String{PRODUCT_ID,".",appname.c_str(),name.c_str()};
			} else if(!strchr(name.c_str(),'.')) {
				name = String{PRODUCT_ID,".",appname.c_str(),".",name.c_str()};
			}
			
			return name.as_quark();

		}

		return String{PRODUCT_ID,".",appname.c_str()}.as_quark();
	}

	void DBus::Service::start() {
		DBus::Error err;
		debug("-----------------------------------------");
		Logger::String{"Listening dbus://",dest}.info(name());
		dbus_bus_request_name(conn, dest, DBUS_NAME_FLAG_REPLACE_EXISTING, err);
		err.verify();
	}

	void DBus::Service::stop() {
		DBus::Error err;
		dbus_bus_release_name(conn, dest, err);
		err.verify();
	}

	static void free_data_block(void *memory) {
		string *xml = ((string *) memory); 
		delete xml;
		debug("Introspection data block was freed");
	}

	DBusHandlerResult DBus::Service::on_message(DBusConnection *connct, DBusMessage *message, DBus::Service *service) noexcept {

		try {

			if(!dbus_message_has_destination(message,service->dest)) {
				return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

			} else if(dbus_message_is_method_call(message, DBUS_INTERFACE_INTROSPECTABLE, "Introspect")) {

				// https://dbus.freedesktop.org/doc/dbus-java/api/org/freedesktop/DBus.Introspectable.html

				std::stringstream xmldata{
					"<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\" " \
					"\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">" \
				};

				xmldata << "<node>";

				for(const auto &interface : service->interfaces) {
					interface.introspect(xmldata);
				}

				xmldata << "</node>";

				{
					static int data_slot = -1;
					if(data_slot == -1) {
						dbus_message_allocate_data_slot(&data_slot);
						debug("------> Got introspection data slot ",data_slot);
					}

					string *xml = new string(xmldata.str().c_str());

					DBusMessage *reply = dbus_message_new_method_return(message);
					dbus_message_set_data(reply,data_slot,xml,free_data_block);

					const char * server_introspection_xml = xml->c_str();
					dbus_message_append_args(reply,DBUS_TYPE_STRING, &server_introspection_xml,DBUS_TYPE_INVALID);
					dbus_connection_send(connct, reply, NULL);
					dbus_message_unref(reply);
				}
				
				return DBUS_HANDLER_RESULT_HANDLED;

			}  else if (dbus_message_is_method_call(message, DBUS_INTERFACE_PROPERTIES, "Get")) {

				// https://dbus.freedesktop.org/doc/dbus-java/api/org/freedesktop/DBus.Properties.html
				DBusMessageIter iter;
				string error{"Invalid argument"};

				if(dbus_message_iter_init(message,&iter)) {

					string args[2];

					for(size_t ix = 0; ix < 2; ix++) {
						if(dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_STRING) {
							DBusBasicValue val;
							dbus_message_iter_get_basic(&iter,&val);
							args[ix] = val.str;
							dbus_message_iter_next(&iter);
						} else {
							throw runtime_error("Invalid argument type");
						}
					}

					// debug("Interface: ",args[0].c_str()," Property: ",args[0].c_str());
					Interface &intf = service->interface(args[0].c_str());

					Logger::String msg{"Property '",args[1].c_str(),"' is invalid for interface '",intf.name(),"'"};
					msg.warning(service->name());
					error = msg;

				}
				
				DBusMessage *response = 
					dbus_message_new_error(
						message,
						DBUS_ERROR_INVALID_ARGS,
						error.c_str()
					);

				dbus_connection_send(connct, response, NULL);
				dbus_message_unref(response);
				dbus_connection_flush(connct);
				return DBUS_HANDLER_RESULT_HANDLED;

			}  else if (dbus_message_is_method_call(message, DBUS_INTERFACE_PROPERTIES, "GetAll")) {

				// https://dbus.freedesktop.org/doc/dbus-java/api/org/freedesktop/DBus.Properties.html
				string error{"Invalid argument"};

				DBusMessageIter iter;
				if(dbus_message_iter_init(message,&iter) && dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_STRING) {

					DBusBasicValue val;
					dbus_message_iter_get_basic(&iter,&val);
					debug("-----------------> GetAll(",val.str,")");
					Interface &intf = service->interface(val.str);

					Logger::String msg{"Interface '",intf.name(),"' doesnt have properties"};
					msg.trace(service->name());
					error = msg;

				} else {

					throw runtime_error("Invalid argument");

				}

				DBusMessage *response = 
					dbus_message_new_error(
						message,
						DBUS_ERROR_INVALID_ARGS,
						error.c_str()
					);

				dbus_connection_send(connct, response, NULL);
				dbus_message_unref(response);
				dbus_connection_flush(connct);
				return DBUS_HANDLER_RESULT_HANDLED;

			} else {

				return service->interface(dbus_message_get_interface(message)).on_message(connct,message,*service);

			}

		} catch(const std::exception &e) {

			Logger::String{e.what()}.error(service->name());

		} catch(...) {

			Logger::String{"Unexpected error processing message"}.error(service->name());

		}
		debug("Returning DBUS_HANDLER_RESULT_NOT_YET_HANDLED for ",dbus_message_get_interface(message)," ",dbus_message_get_member(message));
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

	}

	bool DBus::Service::on_signal(Udjat::DBus::Message &) {
		return false;
	}

	Udjat::Interface & DBus::Service::InterfaceFactory(const XML::Node &node) {

		String intfname;

		for(const char *attrname : { "dbus-interface", "interface", "name" }) {
			String attr{node,attrname};
			if(!attr.empty()) {
				intfname = attr;
				break;
			}

		}

		if(intfname.empty()) {
			intfname = String{PRODUCT_ID,".",Application::Name().c_str()};
		} else if(intfname[0] == '.') {
			intfname = String{PRODUCT_ID,".",Application::Name().c_str(),intfname.c_str()};
		} else if(!strchr(intfname.c_str(),'.')) {
			intfname = String{PRODUCT_ID,".",Application::Name().c_str(),".",intfname.c_str()};
		}

		// Check if interface is already registered.
		for(Interface &interface : interfaces) {
			if(!strcasecmp(intfname.c_str(),interface.interface())) {
				return interface;
			}
		}

		// It's a new interface, insert it.
		return interfaces.emplace_back(node,intfname.as_quark());
	}


 }

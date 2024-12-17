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

	DBus::Service::~Service() {
		dbus_connection_unref(conn);
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

	DBusHandlerResult DBus::Service::on_message(DBusConnection *connct, DBusMessage *message, DBus::Service *service) noexcept {

		try {

			if(!dbus_message_has_destination(message,service->dest)) {
				return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

			} else if(dbus_message_is_method_call(message, DBUS_INTERFACE_INTROSPECTABLE, "Introspect")) {

				// TODO: Implement introspection.
				Udjat::String xmldata{
					"<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\" " \
					"\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">" \
					"<node name=\"",service->dest,"\">"
				};

				for(const auto &interface : service->interfaces) {
					interface.introspect(xmldata);
				}

				xmldata += "</node>";

				debug(xmldata.c_str());

				{
					DBusMessage *reply = dbus_message_new_method_return(message);
					const char * server_introspection_xml = xmldata.c_str();
					dbus_message_append_args(reply,DBUS_TYPE_STRING, &server_introspection_xml,DBUS_TYPE_INVALID);
					dbus_connection_send(connct, reply, NULL);
					dbus_message_unref(reply);
				}
				
				return DBUS_HANDLER_RESULT_HANDLED;

//				Logger::String{"Introspection 'Introspect', wasnt implemented"}.write(Logger::Debug,service->name());
//				return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

			}  else if (dbus_message_is_method_call(message, DBUS_INTERFACE_PROPERTIES, "Get")) {

				// TODO: Implement introspection.
				Logger::String{"Introspection 'Get', wasnt implemented"}.write(Logger::Debug,service->name());
				return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

			}  else if (dbus_message_is_method_call(message, DBUS_INTERFACE_PROPERTIES, "GetAll")) {

				// TODO: Implement introspection.
				Logger::String{"Introspection 'GetAll' wasnt implemented"}.write(Logger::Debug,service->name());
				return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

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

	/// @brief Import value from iter to Udjat::Value
	/// @param iter The iter pointing to input value.
	/// @param value The request element.
	static void import_value(int type, DBusMessageIter *iter, Udjat::Value &value) {

		DBusBasicValue dbval;

		switch(type) {
		case DBUS_TYPE_INVALID:
			throw runtime_error("Required argument not found");

		case DBUS_TYPE_STRING:
			dbus_message_iter_get_basic(iter,&dbval);
			value.set(dbval.str,(Value::Type) value);
			break;

		case DBUS_TYPE_BOOLEAN:
			dbus_message_iter_get_basic(iter,&dbval);
			value.set(dbval.bool_val != 0);
			break;

		case DBUS_TYPE_INT16:
			dbus_message_iter_get_basic(iter,&dbval);
			value.set((int) dbval.i16);
			break;

		case DBUS_TYPE_INT32:
			dbus_message_iter_get_basic(iter,&dbval);
			value.set((int) dbval.i32);
			break;

		case DBUS_TYPE_INT64:
			dbus_message_iter_get_basic(iter,&dbval);
			value.set((int) dbval.i64);
			break;

		case DBUS_TYPE_UINT16:
			dbus_message_iter_get_basic(iter,&dbval);
			value.set((unsigned int) dbval.u16);
			break;

		case DBUS_TYPE_UINT32:
			dbus_message_iter_get_basic(iter,&dbval);
			value.set((unsigned int) dbval.u32);
			break;

		default:
			throw runtime_error("Unexpected argument type");

		}

	}

 }

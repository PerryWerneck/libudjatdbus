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

 #include <udjat/tools/service.h>
 #include <udjat/tools/worker.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/exception.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/interface.h>

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

		if(!dbus_message_has_destination(message,service->dest)) {
			return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

		} else if(dbus_message_is_method_call(message, DBUS_INTERFACE_INTROSPECTABLE, "Introspect")) {

			// TODO: Implement introspection.
			/*
			Udjat::String xmldata{
				"<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\" " \
				"\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n" \
				"<node name=\"",service->dest,"\">\n"
 			};
			*/

			Logger::String{"Introspection 'Introspect', wasnt implemented"}.write(Logger::Debug,service->name());
			return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

		}  else if (dbus_message_is_method_call(message, DBUS_INTERFACE_PROPERTIES, "Get")) {

			// TODO: Implement introspection.
			Logger::String{"Introspection 'Get', wasnt implemented"}.write(Logger::Debug,service->name());
			return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

		}  else if (dbus_message_is_method_call(message, DBUS_INTERFACE_PROPERTIES, "GetAll")) {

			// TODO: Implement introspection.
			Logger::String{"Introspection 'GetAll' wasnt implemented"}.write(Logger::Debug,service->name());
			return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

		} 

		{
			const char *intfname = dbus_message_get_interface(message);
			for(Interface &interface : service->interfaces) {
				debug("Checking '",interface.interface(),"' -> '",intfname,"'");
				if(!strcasecmp(interface.interface(),intfname)) {
					return interface.on_message(connct,message,*service);
				} 
			}
		}

		debug("Returning DBUS_HANDLER_RESULT_NOT_YET_HANDLED for ",dbus_message_get_interface(message)," ",dbus_message_get_member(message));
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

	}

	bool DBus::Service::on_signal(Udjat::DBus::Message &) {
		return false;
	}

	bool DBus::Service::on_method(Udjat::DBus::Message &request, Udjat::Value &response) {

		debug("Method request: '",dbus_message_get_member(request),"'");

		throw DBus::Exception(request,DBUS_ERROR_UNKNOWN_INTERFACE,dbus_message_get_interface(request));

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

	DBus::Service::Interface::Interface(const XML::Node &node, const char *in) 
		: Udjat::Interface{node}, intfname{in} {
		Logger::String("Registering interface ",intfname).trace();
	}

	DBus::Service::Interface::~Interface() {
	}

	bool DBus::Service::Interface::push_back(const XML::Node &node, std::shared_ptr<Action> action) {
		push_back(node).push_back(action);
		return true;
	}

	Udjat::Interface::Handler & DBus::Service::Interface::push_back(const XML::Node &node) {
		return emplace_back(node);
	}

	DBusHandlerResult DBus::Service::Interface::on_message(DBusConnection *connct, DBusMessage *message, DBus::Service &service) {





		//
		// Not found, return error.
		//
		Logger::String{"Cant handle ",dbus_message_get_interface(message),".",dbus_message_get_member(message)}.warning(name());
		DBusMessage *response = 
			dbus_message_new_error(
				message,
				DBUS_ERROR_FAILED,
				String{"Cant find member '",dbus_message_get_member(message),"'"}.c_str()
			);

		dbus_connection_send(connct, response, NULL);
		dbus_message_unref(response);

		return DBUS_HANDLER_RESULT_HANDLED;
	}

 }

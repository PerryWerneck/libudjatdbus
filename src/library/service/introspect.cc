/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2026 Perry Werneck <perry.werneck@gmail.com>
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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/schema.h>
 #include <udjat/tools/service.h>
 #include <udjat/tools/dbus/service.h>
 #include <udjat/tools/interface.h>
 #include <udjat/tools/schema.h>
 #include <udjat/tools/http/schema.h>
 #include <private/tools.h>
 #include <sstream>

 using namespace std;

 namespace Udjat {

	static void free_instrospection_data(void *memory) {
		string *xml = ((string *) memory); 
		delete xml;
		debug("Introspection data block was freed");
	}

	DBusMessage * DBus::Service::introspect(DBusMessage *message) noexcept {

		// https://dbus.freedesktop.org/doc/dbus-java/api/org/freedesktop/DBus.Introspectable.html

		try {

			std::stringstream xmldata;
			xmldata << DBUS_INTROSPECT_1_0_XML_DOCTYPE_DECL_NODE << "<node>";

			Interface::for_each([this,&xmldata](const Interface &interface){

				Schema::Method methods;
				if(!interface.schema(methods)) {
					return false;
				}

				xmldata << "<interface name=\""
						<< this->dest << "." << interface.name()
						<< "\">";

				for(const auto &method : methods) {

					xmldata << "<method name=\"" << MethodNameFactory(method.method()) << "\">";

					Schema::Input in;
					if(interface.schema(method.method(),in)) {
						for(const auto &item : in) {
							xmldata << "<arg name=\"" << item.name() << "\""
									<< "type=\"" << DBus::StringTypeFactory(item.type())
									<< "\" direction=\"in\" />"; 
						}
					}

					Schema::Output out;
					if(interface.schema(method.method(),out)) {
						for(const auto &item : out) {
							xmldata << "<arg name=\"" << item.name() << "\""
									<< "type=\"" << DBus::StringTypeFactory(item.type())
									<< "\" direction=\"out\" />"; 
						}
					}
					
					xmldata << "</method>";

					if(out.options & Schema::Output::Enumerable) {
						xmldata << "<method name=\"GetAll\">";
						xmldata << "<arg name=\"itens\" type=\"a(";
						for(const auto &item : out) {
							xmldata << DBus::StringTypeFactory(item.type());
						}	
						xmldata << ")\" direction=\"out\" /></method>";
					}

					for(const auto &item : out) {
						xmldata << "<property name=\"" << item.name() << "\""
								<< "type=\"" << DBus::StringTypeFactory(item.type()) << "\" access=\"read\"/>";
					}	

				}

				xmldata << "</interface>";

				return false;
			});

			xmldata << "</node>";

			{
				static int data_slot = -1;
				if(data_slot == -1) {
					dbus_message_allocate_data_slot(&data_slot);
					debug("------> Got introspection data slot ",data_slot);
				}

				string *xml = new string(xmldata.str().c_str());
				debug("Introspection data:\n",xml->c_str(),"\n");

				DBusMessage *reply = dbus_message_new_method_return(message);
				dbus_message_set_data(reply,data_slot,xml,free_instrospection_data);

				const char * server_introspection_xml = xml->c_str();
				dbus_message_append_args(reply,DBUS_TYPE_STRING, &server_introspection_xml,DBUS_TYPE_INVALID);

				return reply;
			}
			
		} catch(const std::exception &e) {

			Logger::String{e.what()}.error();
			return dbus_message_new_error(
				message,
				DBUS_ERROR_FAILED,
				e.what()
			);

		}

	}

 }

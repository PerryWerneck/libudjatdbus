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
 #include <dbus/dbus.h>
 #include <udjat/tools/dbus/service.h>
 #include <udjat/tools/interface.h>
 #include <private/request.h>
 #include <udjat/tools/dbus/service.h>

 using namespace std;

 namespace Udjat {

	DBusMessage * DBus::Service::get_property(DBus::Request &request, const char *intf, const char *property_name) noexcept {

		try {

			DBusMessage *reply = nullptr;

			Interface::for_each([this,&reply,&request,intf,property_name](const Interface &interface){

				String name{dest,".",interface.name()};
				debug("Testing '",name.c_str(),"'");

				if(strcmp(name.c_str(),intf)) {
					return false;
				}

				// Found the interface.
				if(!interface.allow(request.role())) {
					reply = dbus_message_new_error(
						(DBusMessage *) request,
						DBUS_ERROR_ACCESS_DENIED,
						"You dont have access to this interface"
					);
					return true;
				}

				Variant value;
				if(interface.get_property(request.path(),property_name,value)) {

					// Got property.
					reply = ReplyFactory((DBusMessage *) request,value);

				} else {

					// Cant get property.
					reply = dbus_message_new_error(
						(DBusMessage *) request,
						DBUS_ERROR_UNKNOWN_PROPERTY,
						String{"Cant find property '",property_name,"' on interface ",interface.name()}.c_str()
					);

				}

				return true;

			});

			if(reply) {
				return reply;
			}

			debug("Message was not processed, returning ",DBUS_ERROR_UNKNOWN_METHOD);
			return dbus_message_new_error(
				(DBusMessage *) request,
				DBUS_ERROR_UNKNOWN_INTERFACE,
				String{"Property '",intf,"' was not recognized by backend"}.c_str()
			);

		} catch(const std::exception &e) {

			Logger::String{e.what()}.error();
			return dbus_message_new_error(
				(DBusMessage *) request,
				DBUS_ERROR_FAILED,
				e.what()
			);

		}

	}

	DBusMessage * DBus::Service::get_properties(DBus::Request &request, const char *intf) noexcept {

		try {

			DBusMessage *reply = nullptr;

			Interface::for_each([this,&reply,&request,intf](const Interface &interface){

				String name{dest,".",interface.name()};
				debug("Testing '",name.c_str(),"'");

				if(strcmp(name.c_str(),intf)) {
					return false;
				}

				// Found the interface.
				if(!interface.allow(request.role())) {
					reply = dbus_message_new_error(
						(DBusMessage *) request,
						DBUS_ERROR_ACCESS_DENIED,
						"You dont have access to this interface"
					);
					return true;
				}

				Variant value;
				if(interface.get_properties(request.path(),value)) {

					// Got property.
					reply = ReplyFactory((DBusMessage *) request,value);

				} else {

					// Cant get property.
					reply = dbus_message_new_error(
						(DBusMessage *) request,
						DBUS_ERROR_UNKNOWN_PROPERTY,
						String{"Interface ",interface.name()," doesnt have properties"}.c_str()
					);

				}

				return true;

			});

			if(reply) {
				return reply;
			}

			debug("Message was not processed, returning ",DBUS_ERROR_UNKNOWN_METHOD);
			return dbus_message_new_error(
				(DBusMessage *) request,
				DBUS_ERROR_UNKNOWN_INTERFACE,
				String{"Property '",intf,"' was not recognized by backend"}.c_str()
			);

		} catch(const std::exception &e) {

			Logger::String{e.what()}.error();
			return dbus_message_new_error(
				(DBusMessage *) request,
				DBUS_ERROR_FAILED,
				e.what()
			);

		}

	}

 }

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

 #include <udjat/tools/schema.h>
 #include <udjat/tools/dbus/service.h>
 #include <udjat/tools/intl.h>

 #include <private/request.h>
 #include <private/response.h>
 #include <private/datatable.h>
 #include <udjat/tools/response.h>
 #include <private/tools.h>
 #include <udjat/tools/interface.h>
 
//  #include <sstream>

//  using namespace std;

 namespace Udjat {

	DBusMessage * DBus::Service::method_call(const Udjat::Interface &interface, const char *name, DBusMessage *message) noexcept {

		try {

			const char *member = dbus_message_get_member(message);

			if(!strcasecmp(member,"GetAll")) {

				// Enumerate children.

				Schema::Output out;
				if(!interface.schema(out)) {
					return dbus_message_new_error(
						message,
						DBUS_ERROR_FAILED,
						String{"The backend does not provide an output schema for ",name}.c_str()
					);
				}

				// Is this interface enumerable?
				if(!(out.options & Schema::Output::Enumerable)) {

					// Interface is not enumerable.
					return dbus_message_new_error(
						message,
						DBUS_ERROR_UNKNOWN_METHOD,
						String{"The interface '",name,"' is not enumerable"}.c_str()
					);

				}
				
				debug("Interface '",name,"' is enumerable, getting results");
				DBus::Request request{conn,message,HTTP::Get};
				DBus::DataTable response{message,out};

				if(!interface.allow(request,response)) {
					return response.MessageFactory();
				}

				debug("Enumerating itens on interface '",interface.name(),"'");
				if(interface.process(request,response)) {
					// The request was processed.
					debug("Sending success");
					return response.MessageFactory();
				} else {
					Logger::String{"Interface '",interface.name(),"' was unable to enumerate itens"}.warning();
				}

			} else {

				// It's a standard method
				
				HTTP::Method method = HTTP::MethodFactory(message);
				if(method == HTTP::UnknownMethod) {
					return dbus_message_new_error(
						message,
						DBUS_ERROR_UNKNOWN_METHOD,
						"The requested method is unknown for this service"
					);
				}

				Schema::Output out;
				if(method == HTTP::Head) {

					out.add(
						Schema::Item{ "statevalue",		Schema::String,		_("The current state")	},
						Schema::Item{ "statemessage",	Schema::String,		_("The current message") }
					);

				} else if(!interface.schema(method,out)) {

					return dbus_message_new_error(
						message,
						DBUS_ERROR_FAILED,
						String{"The backend does not provide an output schema for ",name}.c_str()
					);

				}

				DBus::Request request{conn,message,method};
				if(request.root()) {

					Schema::Input in;
					interface.schema(method,in);

					if(!(in.options & Schema::Input::AllowRoot)) {
						return dbus_message_new_error(
							message,
							DBUS_ERROR_INVALID_ARGS,
							"An object path is required"
						);
					}

				}

				DBus::Response response{message,out};

				if(!interface.allow(request,response)) {
					return response.MessageFactory();
				}
			
				debug("Processing request on interface '",interface.name(),"'");
				if(interface.process(request,response)) {

					debug("APPSTATE: ",response.appstate.value.c_str()," (",response.appstate.message.c_str(),")");

					// The request was processed.
					if(method == HTTP::Head) {
						debug("Status of '",response.appstate.name.c_str(),"' is '",response.appstate.value.c_str(),"' (",response.appstate.message.c_str(),")");

						response["statevalue"] = response.appstate.value.c_str();
						response["statemessage"] = response.appstate.message.c_str();
					}

					debug("Sending reply");
					return response.MessageFactory();
				}

			}

			debug("Message was not processed, returning ",DBUS_ERROR_UNKNOWN_METHOD);
			return dbus_message_new_error(
				message,
				DBUS_ERROR_UNKNOWN_METHOD,
				String{"Method in ",name," was not recognized by backend"}.c_str()
			);


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

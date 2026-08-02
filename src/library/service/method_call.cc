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

//  #include <stdexcept>
//  #include <udjat/tools/intl.h>
//  #include <udjat/tools/exception.h>
//  #include <udjat/tools/variant.h>

//  #include <udjat/tools/service.h>
//  #include <udjat/tools/string.h>
//  #include <udjat/tools/exception.h>
//  #include <udjat/tools/application.h>
//  #include <udjat/tools/interface.h>
//  #include <udjat/tools/timestamp.h>
 #include <udjat/tools/schema.h>

//  #include <udjat/tools/dbus/defs.h>
//  #include <udjat/tools/dbus/connection.h>
//  #include <udjat/tools/dbus/message.h>
 #include <udjat/tools/dbus/service.h>
//  #include <udjat/tools/dbus/exception.h>
//  #include <udjat/tools/datatable.h>

 #include <private/request.h>
 #include <private/response.h>
 #include <private/datatable.h>
 #include <private/tools.h>
 
//  #include <sstream>

//  using namespace std;

 namespace Udjat {

	DBusMessage * DBus::Service::method_call(const Udjat::Interface &interface, const char *name, DBusMessage *message) noexcept {

		try {

			const char *member = dbus_message_get_member(message);

			OutputSchema schema;
			if(!interface.schema(schema)) {
				return dbus_message_new_error(
					message,
					DBUS_ERROR_FAILED,
					String{"The backend does not provide an output schema for ",name}.c_str()
				);
			}

			if(!strcasecmp(member,"GetAll")) {

				// Is this interface enumerable?
				if(!(schema.caps & Schema::Enumerable)) {

					// Interface is not enumerable.
					return dbus_message_new_error(
						message,
						DBUS_ERROR_UNKNOWN_METHOD,
						String{"The interface '",name,"' is not enumerable"}.c_str()
					);

				}
				
				debug("Interface '",name,"' is enumerable, getting results");
				DBus::Request request{message,HTTP::Get,dbus_message_get_path(message)};
				DBus::DataTable response{message,schema};

				debug("Enumerating itens on interface '",interface.name(),"'");
				if(interface.process(request,response)) {
					// The request was processed.
					if(response.code == HTTP::Ok) {
						debug("Sending success");
						return response.MessageFactory();
					}

					debug("Sending error");
					return dbus_message_new_error(
						message,
						ErrorFactory(response.code),
						response.body.empty() ? response.message.c_str() : response.body.c_str()
					);

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

				DBus::Request request{message,method,dbus_message_get_path(message)};
				if(request.root()) {
					return dbus_message_new_error(
						message,
						DBUS_ERROR_INVALID_ARGS,
						"An object path is required"
					);
				}

				DBus::Response response{message,schema};
			
				debug("Processing request on interface '",interface.name(),"'");
				if(interface.process(request,response)) {
					// The request was processed.
					debug("Sending enumeration reply");
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

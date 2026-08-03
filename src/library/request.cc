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
 #include <private/request.h>
 #include <udjat/tools/request.h>
 #include <private/tools.h>
 #include <udjat/tools/request.h>
 #include <udjat/tools/schema.h>
 #include <udjat/tools/interface.h>
 #include <udjat/tools/http/method.h>
 #include <udjat/authentication.h>
 #include <dbus/dbus.h>
 #include <cstring>
 #include <memory>

 using namespace std;

 namespace Udjat {

	DBus::Request::Request(DBusConnection *conn, DBusMessage *m, const HTTP::Method h) : Udjat::Request{dbus_message_get_path(m)}, message{m}, request_method{h} {

		dbus_message_ref(message);

		// Build authentication
		{
			const char *sender = dbus_message_get_sender(message);
			if(sender) {

				DBusError err;
				dbus_error_init(&err);

				unsigned long uid = dbus_bus_get_unix_user(conn, sender, &err);
				if (dbus_error_is_set(&err)) {

					Logger::String{"Unable to get authentication for ",sender,": ",err.message}.warning();
					dbus_error_free(&err);

				} else {

					if(uid == 0) {
						// Root user.
					} else {
						// Regular user.
#ifdef DEBUG
						auth = make_shared<Authentication>(sender,Authentication::Owner);
#else
						auth = make_shared<Authentication>(sender,Authentication::Member);
#endif // DEBUG					
					}

				}

			}
		}

	}

	DBus::Request::~Request() {
		dbus_message_unref(message);
	}

	HTTP::Method DBus::Request::method() const noexcept {
		return request_method;
	}

	DBusMessage * DBus::Request::parse_input(const Udjat::Interface &intf) {

		Schema::Input schema;
		if(!intf.schema(path(),schema)) {
			return nullptr;
		}

		// TODO: Implement input schema parser.


		return dbus_message_new_error(
			message,
			DBUS_ERROR_NOT_SUPPORTED,
			"Input parser is incomplete"
		);

	}


 }


 


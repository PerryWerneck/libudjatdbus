/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2021 Perry Werneck <perry.werneck@gmail.com>
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

 #include "private.h"
 #include <udjat/worker.h>

 namespace Udjat {

	namespace DBus {

		DBusHandlerResult Connection::filter(DBusConnection *connection, DBusMessage *message, Connection *controller) noexcept {

			if(!(strcasecmp("Introspect",dbus_message_get_member(message)) || strcasecmp("org.freedesktop.DBus.Introspectable",dbus_message_get_interface(message)))) {

				// https://dbus.freedesktop.org/doc/dbus-specification.html#standard-interfaces-introspectable
				// https://dbus.freedesktop.org/doc/dbus-specification.html#introspection-format

				// TODO: How to implement introspection?

#ifdef DEBUG
				cout	<< "Client is requesting object introspection" << endl;
#endif // DEBUG


				return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
			}

#ifdef DEBUG
			cout	<< "Member:    " << dbus_message_get_member(message) << endl
					<< "Interface: " << dbus_message_get_interface(message) << endl;
#endif // DEBUG

			try {
				//
				// Search for internal workers.
				//
				Worker * worker = controller->find(message);

				if(worker) {
					DBus::Request request(message,worker->getAction(message));
					DBus::Response response;
					if(worker->work(request, response)) {
						response.reply(connection,message);
						return DBUS_HANDLER_RESULT_HANDLED;
					}
				}

			} catch(const std::exception &e) {

				cerr << "D-Bus\t" << e.what() << endl;

				DBusMessage * rsp = dbus_message_new_error(message,DBUS_ERROR_FAILED,e.what());
				dbus_connection_send(connection, rsp, NULL);
				dbus_message_unref(rsp);
				return DBUS_HANDLER_RESULT_HANDLED;

			}

			return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

		}

	}

 }


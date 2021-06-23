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

 namespace Udjat {

	namespace DBus {

		DBusHandlerResult Connection::filter(DBusConnection *connection, DBusMessage *message, Connection *controller) noexcept {

			DBusHandlerResult rc = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

			for(auto worker : controller->workers) {

				if(worker->equal(message)) {

					// Found worker for this message.
					try {

						worker->work(controller, message);

					} catch(const exception &e) {

						cerr << "D-Bus\t" << e.what() << endl;

						if(dbus_message_get_type(message) == DBUS_MESSAGE_TYPE_METHOD_CALL) {

							// Send error response
							DBusMessage * rsp = dbus_message_new_error(message,DBUS_ERROR_FAILED,e.what());
							dbus_connection_send(connection, rsp, NULL);
							dbus_message_unref(rsp);

						}

					}

					rc = DBUS_HANDLER_RESULT_HANDLED;
				}

			}

			return rc;

		}

	}

 }


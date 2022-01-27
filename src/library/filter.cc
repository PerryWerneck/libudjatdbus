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

 #include <config.h>
 #include <udjat/tools/dbus.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/worker.h>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	DBusHandlerResult DBus::Connection::on_signal(DBusMessage *message) {

		const char *member		= dbus_message_get_member(message);
		const char *interface	= dbus_message_get_interface(message);

#ifdef DEBUG
		cout << name << "\tsignal(" << interface << " " << member << ")" << endl;
#endif // DEBUG

		lock_guard<recursive_mutex> lock(guard);

		try {

			for(auto intf : interfaces) {
				if(!intf.name.compare(interface)) {
					for(auto memb : intf.members) {
						if(!memb.name.compare(member)) {

							try {

								Message msg(message);
								memb.call(msg);

							} catch(const exception &e) {

								cerr << name << "\tError '" << e.what() << "' processing signal " << interface << "." << member << endl;

							} catch(...) {

								cerr << name << "\tUnexpected error processing signal " << interface << "." << member << endl;

							}

						}

					}
					break;
				}
			}

		} catch(const exception &e) {

			cerr << name << "\t" << interface << " " << member << ": " << e.what() << endl;

		} catch(...) {

			cerr << name << "\t" << interface << " " << member << ": Unexpected error" << endl;

		}

		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

	}

	DBusHandlerResult DBus::Connection::filter(DBusConnection *dbc, DBusMessage *message, DBus::Connection *connection) {

		if(dbus_message_get_type(message) == DBUS_MESSAGE_TYPE_SIGNAL) {
			return connection->on_signal(message);
		}

		// TODO: Filter method calls.

		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}


 }


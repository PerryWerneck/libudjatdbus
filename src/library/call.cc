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
 #include "private.h"

 using namespace std;

 namespace Udjat {


	/// @brief Parameters for method call.
	struct CallParameters {

		DBus::Connection *connection;
		const std::function<void(DBus::Message &)> call;

		CallParameters(DBus::Connection *c, const std::function<void(DBus::Message &)> &f) : connection(c), call(f) {
		}

	};

	static void dbus_call_reply(DBusPendingCall *pending, CallParameters *parameters) {

		DBusError error;
		dbus_error_init(&error);

		if(!dbus_pending_call_get_completed(pending)) {

			dbus_set_error_const(&error, "Unexpected", "Failed to get pending reply");
			DBus::Message message(error);
			parameters->call(message);
			dbus_pending_call_cancel(pending);

		} else {

			// Got response
			DBusMessage * message = dbus_pending_call_steal_reply(pending);

			if(message) {

				try {

					DBus::Message msg(message);
					parameters->call(msg);

				} catch(const std::exception &e) {

					cerr << "dbus\tError '" << e.what() << "' processing response to d-bus method call" << endl;

				} catch(...) {

					cerr << "dbus\tUnexpected error processing response to d-bus method call" << endl;

				}

				dbus_message_unref(message);

			} else {

				dbus_set_error_const(&error, "empty", "No response");

				try {

					DBus::Message msg(error);
					parameters->call(msg);

				} catch(const std::exception &e) {

					cerr << "dbus\tError '" << e.what() << "' processing response to d-bus method call" << endl;

				} catch(...) {

					cerr << "dbus\tUnexpected error processing response to d-bus method call" << endl;

				}
			}

		}

		dbus_error_free(&error);

		delete parameters;
	}

	void DBus::Connection::call(DBusMessage * message, std::function<void(Message & message)> call) {

		DBusPendingCall *pending = NULL;

		if(!dbus_connection_send_with_reply(connection,message,&pending,DBUS_TIMEOUT_USE_DEFAULT)) {
			throw std::runtime_error("Can't send DBus method call");
		}

		CallParameters *parameters = new CallParameters(this,call);

		if(!dbus_pending_call_set_notify(pending, (DBusPendingCallNotifyFunction) dbus_call_reply, (void *) parameters, NULL)) {
			dbus_pending_call_unref(pending);
			dbus_message_unref(message);
			delete parameters;
			throw std::runtime_error("Can't set call notify function");
		}

		dbus_pending_call_unref(pending);

	}

	void DBus::Connection::call(const Message &message, std::function<void(Message & message)> call) {
		DBusMessage *msg = (DBusMessage *) message;
		if(!msg) {
			throw runtime_error("Empty D-Bus message");
		}
		this->call(msg,call);
	}

	void DBus::Connection::call(const char *destination,const char *path, const char *interface, const char *member, std::function<void(DBus::Message & message)> call) {

		DBusMessage * message = dbus_message_new_method_call(destination,path,interface,member);
		if(message == NULL) {
			throw std::runtime_error("Error creating DBus method call");
		}

		this->call(message,call);

		dbus_message_unref(message);

	}

 }



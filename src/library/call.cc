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
 #include <udjat/tools/logger.h>
 #include "private.h"

 using namespace std;

 namespace Udjat {


	/// @brief Parameters for method call.
	struct CallParameters {

		DBusConnection * connection;
		const std::function<void(DBus::Message &)> call;

		CallParameters(DBus::Connection *c, const std::function<void(DBus::Message &)> &f) : connection(c->getConnection()), call(f) {
			Logger::trace() << "New call parameters " << hex << ((void *) this) << dec << endl;
			dbus_connection_ref(connection);
		}

		~CallParameters() {
			Logger::trace() << "Delete call parameters " << hex << ((void *) this) << dec << endl;
			dbus_connection_unref(connection);
		}

	};

	static void dbus_call_reply(DBusPendingCall *pending, CallParameters *parameters) {

		debug("Got a reply from pending call");

		DBusError error;
		dbus_error_init(&error);

		if(!dbus_pending_call_get_completed(pending)) {

			// NO response

			debug("No response from d-bus call");

			dbus_set_error_const(&error, "Unexpected", "Failed to get pending reply");
			DBus::Message message(error);

			try {

				parameters->call(message);

			} catch(std::exception &e) {

				cerr << "dbus\tCan't process error message: " << e.what() << endl;

			} catch(...) {

				cerr << "dbus\tUnexpected error processing error message" << endl;

			}

			dbus_pending_call_cancel(pending);

		} else {

			// Got response
			DBusMessage * message = dbus_pending_call_steal_reply(pending);

			if(message) {

				debug("Got response from dbus call");

				try {

					DBus::Message msg{message};
					parameters->call(msg);

				} catch(std::exception &e) {

					cerr << "dbus\tCan't process error message: " << e.what() << endl;

				} catch(...) {

					cerr << "dbus\tUnexpected error processing error message" << endl;

				}

				dbus_message_unref(message);

			} else {

				debug("Empty response from dbus call");
				dbus_set_error_const(&error, "empty", "No response");

				try {

					DBus::Message msg(error);
					parameters->call(msg);

				} catch(std::exception &e) {

					cerr << "dbus\tCan't process error message: " << e.what() << endl;

				} catch(...) {

					cerr << "dbus\tUnexpected error processing error message" << endl;

				}

			}

		}

		dbus_error_free(&error);

	}

	void DBus::Connection::call(DBusMessage * message) {

		if(!connection) {
			throw runtime_error("D-Bus connection is not available");
		}

		DBusError error;
		dbus_error_init(&error);

		DBusMessage * response =
			dbus_connection_send_with_reply_and_block(
				connection,
				message,
				DBUS_TIMEOUT_USE_DEFAULT,
				&error
			);

		if(response) {
			dbus_message_unref(response);
		}

		if(dbus_error_is_set(&error)) {

			string message{error.message};
			dbus_error_free(&error);
			throw runtime_error(message);

		}

	}

	void DBus::Connection::call_and_wait(DBusMessage * message, const std::function<void(Message & message)> &call) {

		if(!connection) {
			throw runtime_error("D-Bus connection is not available");
		}

		DBusError error;
		dbus_error_init(&error);

		DBusMessage * response =
			dbus_connection_send_with_reply_and_block(
				connection,
				message,
				DBUS_TIMEOUT_USE_DEFAULT,
				&error
			);

		if(dbus_error_is_set(&error)) {

			DBus::Message message{error};

			try {

				call(message);

			} catch(...) {

				dbus_error_free(&error);
				throw;
			}

			dbus_error_free(&error);

		} else if(response) {

			DBus::Message message{response};

			try {

				call(message);

			} catch(...) {

				dbus_message_unref(response);
				throw;
			}

			dbus_message_unref(response);

		}

	}

	void free_parameters(CallParameters *parameters) {
		debug("Cleaning pending call");
		delete parameters;
	}

	void DBus::Connection::call(DBusMessage * message, const std::function<void(Message & message)> &call) {

		if(!connection) {
			throw runtime_error("D-Bus connection is not available");
		}

		debug("----------------------------------- pending call");

		DBusPendingCall *pending = NULL;

		if(!dbus_connection_send_with_reply(connection,message,&pending,DBUS_TIMEOUT_USE_DEFAULT)) {
			throw std::runtime_error("Can't send DBus method call");
		}

		if(!pending) {
			throw std::runtime_error("Invalid 'pending call' handler");
			return;
		}

		CallParameters *parameters = new CallParameters(this,call);

		static dbus_int32_t slot = -1;
		if(slot == -1) {
			if(!dbus_pending_call_allocate_data_slot(&slot)) {
				throw std::runtime_error("Cant allocate pending call data slot");
			}
		}

		dbus_pending_call_set_data(pending,slot,parameters,(DBusFreeFunction) free_parameters);

		if(!dbus_pending_call_set_notify(pending, (DBusPendingCallNotifyFunction) dbus_call_reply, (void *) parameters, NULL)) {
			dbus_pending_call_unref(pending);
			dbus_message_unref(message);
			delete parameters;
			throw std::runtime_error("Can't set call notify function");
		}

		dbus_pending_call_unref(pending);

		debug("------------------------------------ Pending call was set");

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



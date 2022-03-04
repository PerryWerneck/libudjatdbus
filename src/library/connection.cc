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
 #include <udjat/tools/dbus.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/worker.h>
 #include <iostream>
 #include <unistd.h>
 #include <pthread.h>

 using namespace std;

 namespace Udjat {

	std::recursive_mutex DBus::Connection::guard;

	DBus::Connection & DBus::Connection::getInstance() {
		if(getuid() == 0) {
			return getSystemInstance();
		}
		return getSessionInstance();
	}

	static DBusConnection * ConnectionFactory(DBusBusType type) {

		DBusError err;
		dbus_error_init(&err);

		DBusConnection * connct = dbus_bus_get(type, &err);
		if(dbus_error_is_set(&err)) {
			std::string message(err.message);
			dbus_error_free(&err);
			throw std::runtime_error(message);
		}

		return connct;
	}

	DBus::Connection & DBus::Connection::getSystemInstance() {
		lock_guard<recursive_mutex> lock(guard);
		static DBus::Connection instance(ConnectionFactory(DBUS_BUS_SYSTEM),"sysbus");
		return instance;
	}

	DBus::Connection & DBus::Connection::getSessionInstance() {
		lock_guard<recursive_mutex> lock(guard);
		static DBus::Connection instance(ConnectionFactory(DBUS_BUS_SESSION),"userbus");
		return instance;
	}

	DBus::Connection::Connection(DBusConnection * c, const char *n, bool reg) : name(n), connection(c) {

		static bool initialized = false;
		if(!initialized) {
			cout << name << "\tInitializing thread system" << endl;
			dbus_threads_init_default();
		}

		lock_guard<recursive_mutex> lock(guard);

		try {

			if(reg) {
				// Register
				DBusError err;
				dbus_error_init(&err);

				dbus_bus_register(connection,&err);
				if(dbus_error_is_set(&err)) {
					std::string message(err.message);
					dbus_error_free(&err);
					throw std::runtime_error(message);
				}
			}

			if (dbus_connection_add_filter(connection, (DBusHandleMessageFunction) filter, this, NULL) == FALSE) {
				throw std::runtime_error("Cant add filter to D-Bus connection");
			}

			// Não encerro o processo ao desconectar.
			dbus_connection_set_exit_on_disconnect(connection, false);

			if(use_thread) {
				//
				// Thread mode
				//
				thread = new std::thread([this] {

					pthread_setname_np(pthread_self(),name.c_str());

					cout << name << "\tService thread begin" << endl;
					auto connct = connection;
					dbus_connection_ref(connct);
					while(connection && dbus_connection_read_write(connct,100)) {
						dispatch(connct);
					}
					cout << name << "\tFlushing connection" << endl;
					dbus_connection_flush(connct);
					dbus_connection_unref(connct);
					cout << name << "\tService thread end" << endl;

				});

			} else {
				//
				// Non thread mode
				//
				if(!dbus_connection_set_watch_functions(
					connection,
					(DBusAddWatchFunction) add_watch,
					(DBusRemoveWatchFunction) remove_watch,
					(DBusWatchToggledFunction) toggle_watch,
					this,
					nullptr)
				) {
					throw runtime_error("dbus_connection_set_watch_functions has failed");
				}

				if(!dbus_connection_set_timeout_functions(
					connection,
					(DBusAddTimeoutFunction) add_timeout,
					(DBusRemoveTimeoutFunction) remove_timeout,
					(DBusTimeoutToggledFunction) toggle_timeout,
					this,
					nullptr)
				) {
					throw runtime_error("dbus_connection_set_timeout_functions has failed");
				}

			}

		} catch(...) {

			dbus_connection_unref(connection);
			throw;

		}

	}

	static DBusConnection * ConnectionFactory(const char *busname) {

		if(!(busname && *busname)) {
			throw system_error(EINVAL,system_category(),"Invalid busname");
		}

		DBusError err;
		dbus_error_init(&err);

		cout << "d-bus\tOpening '" << busname << "'" << endl;
		DBusConnection * connection = dbus_connection_open(busname, &err);
		if(dbus_error_is_set(&err)) {
			std::string message(err.message);
			dbus_error_free(&err);
			throw std::runtime_error(message);
		}

		return connection;

	}

	DBus::Connection::Connection(const char *busname, const char *name) : Connection(ConnectionFactory(busname),name) {
	}

	DBus::Connection::~Connection() {

		cout << name << "\tConnection destroyed" << endl;
		// Remove listeners.
		interfaces.remove_if([this](Interface &intf) {
			intf.remove_from(this);
			return true;
		});

		// Remove filter
		dbus_connection_remove_filter(connection,(DBusHandleMessageFunction) filter, this);

		// Stop D-Bus connection
		if(thread) {

			dbus_connection_unref(connection);
			connection = nullptr;
			cout << name << "\tWaiting for service thread" << endl;
			thread->join();
			delete thread;

		} else if(!use_thread) {

			cout << name << "\tRestoring d-bus watchers" << endl;

			if(!dbus_connection_set_watch_functions(
				connection,
				(DBusAddWatchFunction) NULL,
				(DBusRemoveWatchFunction) NULL,
				(DBusWatchToggledFunction) NULL,
				this,
				nullptr)
			) {
				cerr << "dbus\tdbus_connection_set_watch_functions has failed" << endl;
			}

			if(!dbus_connection_set_timeout_functions(
				connection,
				(DBusAddTimeoutFunction) NULL,
				(DBusRemoveTimeoutFunction) NULL,
				(DBusTimeoutToggledFunction) NULL,
				NULL,
				nullptr)
			) {
				cerr << "dbus\tdbus_connection_set_timeout_functions has failed" << endl;
			}

			dbus_connection_unref(connection);
			connection = nullptr;
		}

		Udjat::MainLoop::getInstance().remove(this);

	}

	/*
	/// @brief Passagem de parâmetros para método D-Bus.
	struct MethodData {
		std::function<void(DBusMessage * message, DBusError *error)> call;
	};

	static void dbus_call_reply(DBusPendingCall *pending, void *user_data) {

		MethodData *data = (MethodData *) user_data;

		DBusError error;
		dbus_error_init(&error);

		if(!dbus_pending_call_get_completed(pending)) {

			dbus_set_error_const(&error, "Failed", "DBus Error");
			data->call(nullptr,&error);

			dbus_pending_call_cancel(pending);


		} else {

			// Got response
			DBusMessage * message = dbus_pending_call_steal_reply(pending);

			if(message) {
				data->call(message,&error);
				dbus_message_unref(message);
			} else {
				dbus_set_error_const(&error, "empty", "No response");
				data->call(message,&error);
			}

		}

		dbus_error_free(&error);

		delete data;
	}

	void DBus::Connection::call(const char *destination,const char *path, const char *interface, const char *member, std::function<void(DBusMessage * message, DBusError *error)> call) {

		DBusMessage * message = dbus_message_new_method_call(destination,path,interface,member);
		if(message == NULL) {
			throw std::runtime_error("Can't create DBus method call");
		}

		DBusPendingCall *pc = NULL;

		if(!dbus_connection_send_with_reply(connection,message,&pc,DBUS_TIMEOUT_USE_DEFAULT)) {
			throw std::runtime_error("Can't send DBus method call");
		}

		MethodData * data = new MethodData();
		data->call = call;

		if(!dbus_pending_call_set_notify(pc, dbus_call_reply, data, NULL)) {
			dbus_pending_call_unref(pc);
			dbus_message_unref(message);
			delete data;
			throw std::runtime_error("Can't set notify method");
		}

		dbus_pending_call_unref(pc);
		dbus_message_unref(message);

	}
	*/


 }


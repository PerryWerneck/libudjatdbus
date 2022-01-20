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
 #include <udjat/worker.h>
 #include <iostream>

 using namespace std;

 /*
#include <ipc/dbus.h>
#include <stdexcept>
#include <cstring>
#include <fstream>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <ext/stdio_filebuf.h>
#include <systemd/sd-login.h>

using std::clog;
using std::endl;
using std::cout;
using std::string;
using std::cerr;
*/

 namespace Udjat {

	std::recursive_mutex DBus::Connection::guard;

	DBus::Connection & DBus::Connection::getSystemInstance() {
		lock_guard<recursive_mutex> lock(guard);
		static DBus::Connection instance(DBUS_BUS_SYSTEM);
		return instance;
	}

	DBus::Connection & DBus::Connection::getSessionInstance() {
		lock_guard<recursive_mutex> lock(guard);
		static DBus::Connection instance(DBUS_BUS_SESSION);
		return instance;
	}

	DBus::Connection::Connection() {
	}

	DBus::Connection::Connection(DBusConnection * connection) : Connection() {

		// Register
		DBusError err;
		dbus_error_init(&err);

		dbus_bus_register(connection,&err);
		if(dbus_error_is_set(&err)) {
			std::string message(err.message);
			dbus_error_free(&err);
			throw std::runtime_error(message);
		}

		// Set!
		set(connection);

	}

	DBus::Connection::Connection(DBusBusType type) : Connection() {

		DBusError err;
		dbus_error_init(&err);

		DBusConnection * connct = dbus_bus_get(type, &err);
		if(dbus_error_is_set(&err)) {
			std::string message(err.message);
			dbus_error_free(&err);
			throw std::runtime_error(message);
		}

		set(connct);

	}


	DBus::Connection::Connection(const char *busname) : Connection() {

		DBusError err;
		dbus_error_init(&err);

		DBusConnection * connection = dbus_connection_open(busname, &err);
		if(dbus_error_is_set(&err)) {
			std::string message(err.message);
			dbus_error_free(&err);
			throw std::runtime_error(message);
		}

		try {

			dbus_error_init(&err);

			dbus_bus_register(connection,&err);
			if(dbus_error_is_set(&err)) {
				std::string message(err.message);
				dbus_error_free(&err);
				throw std::runtime_error(message);
			}

			set(connection);

		} catch(...) {
			dbus_connection_unref(connection);
			throw;
		}

	}

	void DBus::Connection::set(DBusConnection * connection) {

		lock_guard<recursive_mutex> lock(guard);
		if(this->connection) {
			std::runtime_error("Can't change connection handle");
		}

		this->connection = connection;

		if (dbus_connection_add_filter(connection, (DBusHandleMessageFunction) filter, this, NULL) == FALSE) {
			dbus_connection_unref(connection);
			throw std::runtime_error("Erro ao ativar filtro de mensagens D-Bus");
		}

		// Não encerro o processo ao desconectar.
		dbus_connection_set_exit_on_disconnect(connection, false);
	}

	void DBus::Connection::removeMatch(DBus::Connection::Interface &intf) {
		DBusError error;
		dbus_error_init(&error);

		dbus_bus_remove_match(connection,intf.getMatch().c_str(), &error);
		dbus_connection_flush(connection);

		if (dbus_error_is_set(&error)) {
			std::cerr << "d-bus\t" << error.message << std::endl;
		}
	}

	DBus::Connection::~Connection() {

		cout << "d-bus\tConnection destroyed" << endl;

		active = false;

		// Remove listeners.
		interfaces.remove_if([this](Interface &intf) {
			removeMatch(intf);
			return true;
		});

		if(connection) {
			// Remove filter
			dbus_connection_remove_filter(connection,(DBusHandleMessageFunction) filter, this);

			// Stop D-Bus connection
			dbus_connection_unref(connection);

			connection = nullptr;
		}

		if(thread) {
			// Wait for d-bus thread
			cout << "d-bus\tWaiting for service thread" << endl;
			thread->join();
		}

	}

	/// @brief Subscribe to D-Bus signal.
	void DBus::Connection::subscribe(void *id, const char *interface, const char *member, std::function<void(DBus::Message &message)> call) {

		lock_guard<recursive_mutex> lock(guard);
		getInterface(interface).members.emplace_back(id,member,call);

		if(!active) {
			start();
		}
	}

	void DBus::Connection::unsubscribe(void *id, const char *interface, const char *memberName) {

		lock_guard<recursive_mutex> lock(guard);

		if(getInterface(interface).unsubscribe(id,memberName)) {

			interfaces.remove_if([this](Interface &interface ){
				if(interface.empty()) {
					removeMatch(interface);
					return true;
				}
				return false;
			});

		}

	}

	void DBus::Connection::unsubscribe(void *id) {

		lock_guard<recursive_mutex> lock(guard);
		interfaces.remove_if([this,id](Interface &interface){
			if(interface.unsubscribe(id)) {
				removeMatch(interface);
				return true;
			}
			return false;
		});
	}

	void DBus::Connection::start() {

		lock_guard<recursive_mutex> lock(guard);

		if(thread)
			return;

		active = true;
		thread = new std::thread([this]{

#ifdef DEBUG
			cout << "d-bus\tService thread begin" << endl;
#endif // DEBUG

			while(active && connection && dbus_connection_read_write(connection,500)) {
				while(connection && dbus_connection_get_dispatch_status(connection) == DBUS_DISPATCH_DATA_REMAINS) {
					dbus_connection_dispatch(connection);
				}
			}

			active = false;

			{
				lock_guard<recursive_mutex> lock(guard);
				if(thread) {
					thread->detach();
					delete thread;
					thread = nullptr;
				}
			}

#ifdef DEBUG
			cout << "d-bus\tService thread end" << endl;
#endif // DEBUG

		});


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


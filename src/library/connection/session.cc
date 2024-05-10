/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
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

 /**
  * @brief Implements session bus connection.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <dbus/dbus.h>
 #include <udjat/tools/dbus/connection.h>
 #include <udjat/tools/logger.h>
 #include <private/dataslot.h>
 #include <private/mainloop.h>

 using namespace std;

 namespace Udjat {

	static DBusConnection *connct = NULL;
	static size_t refcount = 0;

	static void trace_connection_free(DBusConnection **connection) {
		if(refcount) {
			Logger::String("Session bus '",((unsigned long) *connection),"' was released with ",refcount," references").warning("d-bus");
		} else {
			Logger::String("Session bus '",((unsigned long) *connection),"' was released").trace("d-bus");
		}
		*connection = nullptr;
	}

	DBusConnection * DBus::SessionBus::ConnectionFactory() {

		lock_guard<mutex> lock(guard);

		if(connct) {
			refcount++;
			dbus_connection_ref(connct);
			return connct;
		}

		Logger::String{"Opening shared connection to session bus"}.trace("d-bus");

		// https://github.com/dbus-cxx/dbus-cxx/blob/master/dbus-cxx/connection.cpp

		DBusError err;
		dbus_error_init(&err);

		connct = dbus_bus_get(DBUS_BUS_SESSION, &err);
		if(dbus_error_is_set(&err)) {
			std::string message(err.message);
			dbus_error_free(&err);
			throw std::runtime_error(message);
		}

		// Setup connection.
		dbus_connection_set_data(connct,DataSlot::getInstance().value(),&connct,(DBusFreeFunction) trace_connection_free);
		mainloop_add(connct);

		refcount++;
		return connct;

	}

	DBus::SessionBus::SessionBus() : Abstract::DBus::Connection{"SysBUS",SessionBus::ConnectionFactory()} {
	}

	DBus::SessionBus::~SessionBus() {
		lock_guard<mutex> lock(guard);
		refcount--;

		debug("SessionBus refcount is ",refcount);
		if(!refcount) {
			mainloop_remove(connct);
			connct = NULL;
		}
	}

	DBus::SessionBus & DBus::SessionBus::getInstance() {
		static DBus::SessionBus instance;
		return instance;
	}

 }

  /*
 #include <config.h>
 #include <udjat/defs.h>
 #include <dbus/dbus.h>
 #include <udjat/tools/dbus/connection.h>

 namespace Udjat {

	static DBusConnection * SessionConnectionFactory() {

		#error refactor

		Udjat::DBus::initialize();

		DBusError err;
		dbus_error_init(&err);

		DBusConnection * connct = dbus_bus_get(DBUS_BUS_SESSION, &err);
		if(dbus_error_is_set(&err)) {
			std::string message(err.message);
			dbus_error_free(&err);
			throw std::runtime_error(message);
		}

		return connct;

	}

	DBus::SessionBus::SessionBus() : Abstract::DBus::Connection{"SessionBUS",SessionConnectionFactory()} {
		bind();
		open();
	}

	DBus::SessionBus::~SessionBus() {
		close();
		unbind();
		dbus_connection_unref(conn);
	}

	DBus::SessionBus & DBus::SessionBus::getInstance() {
		static DBus::SessionBus instance;
		return instance;
	}

 }

*/

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
  * @brief Implements named bus connection.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <dbus/dbus.h>
 #include <udjat/tools/dbus/connection.h>
 #include <udjat/tools/logger.h>
 #include <private/mainloop.h>
 #include <private/dataslot.h>

 using namespace std;

 namespace Udjat {


	static DBusConnection * NamedConnectionFactory(const char *address) {

		if(!(address && *address)) {
			throw runtime_error("Empty d-bus address");
		}

		DBusError err;
		dbus_error_init(&err);

		DBusConnection *connection = dbus_connection_open_private(address, &err);
		if(dbus_error_is_set(&err)) {
			Logger::String message{"Cant open '",address,"': ",err.message};
			dbus_error_free(&err);
			throw runtime_error(message);
		}

		return connection;

	}

	static void trace_connection_free(const Abstract::DBus::Connection *connection) {
		Logger::String("Named connection '",((unsigned long) connection),"' was released").trace("d-bus");
	}

	DBus::NamedBus::NamedBus(const char *name, DBusConnection * conn) : Abstract::DBus::Connection{name,conn} {

		if(Logger::enabled(Logger::Trace)) {
			dbus_connection_set_data(conn,DataSlot::getInstance().value(),this,(DBusFreeFunction) trace_connection_free);
		}

		mainloop_add(conn);
		bus_register();

	}

	DBus::NamedBus::NamedBus(const char *connection_name, const char *address) : NamedBus{connection_name,NamedConnectionFactory(address)} {
	}

	DBus::NamedBus::~NamedBus() {

		{
			clear();
		}

		{
			lock_guard<mutex> lock(guard);

			// Disconnect from mainloop
			mainloop_remove(conn);

			// Close connection.
			dbus_connection_flush(conn);
			dbus_connection_close(conn);
		}

	}

 }


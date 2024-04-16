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

 using namespace std;

 namespace Udjat {

	static DBusConnection * NamedConnectionFactory(const char *bus_name) {

		DBusError err;
		dbus_error_init(&err);

		DBusConnection *connection = dbus_connection_open_private(bus_name, &err);
		if(dbus_error_is_set(&err)) {
			Logger::String message{"Cant open '",bus_name,"': ",err.message};
			dbus_error_free(&err);
			throw runtime_error(message);
		}

		return connection;

	}

	DBus::NamedBus::NamedBus(const char *connection_name, const char *bus_name) : Abstract::DBus::Connection{connection_name,NamedConnectionFactory(bus_name)} {

		try {

			open();
			bus_register();

			int fd = -1;
			if(dbus_connection_get_socket(conn,&fd)) {
				Logger::String("Got connection to '",connection_name,"' on socket '",fd,"'").trace("d-bus");
			} else {
				Logger::String("Got connection to '",connection_name,"'").trace("d-bus");
			}

		} catch(...) {

			if(conn) {
				Logger::String{"Closing connection to '",connection_name,"' due to initialization error"}.error("d-bus");
				dbus_connection_close(conn);
				dbus_connection_unref(conn);
				conn = nullptr;
			}

			throw;

		}

	}

	DBus::NamedBus::~NamedBus() {

		int fd = -1;
		if(dbus_connection_get_socket(conn,&fd)) {
			Logger::String("Closing named connection '",name(),"' on socket '",fd,"'").trace("d-bus");
		} else {
			Logger::String("Closing named connection '",name(),"'").trace("d-bus");
		}

		close();
		dbus_connection_close(conn);
		dbus_connection_unref(conn);
	}

 }


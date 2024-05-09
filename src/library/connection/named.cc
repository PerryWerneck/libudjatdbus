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

	DBus::NamedBus::NamedBus(const char *connection_name, DBusConnection * conn) : Abstract::DBus::Connection{connection_name,conn} {

		try {

			open();

			int fd = -1;
			if(dbus_connection_get_socket(conn,&fd)) {
				Logger::String("Got private connection ",((unsigned long) this)," on socket '",fd,"'").write(Logger::Debug,connection_name);
			} else {
				Logger::String("Got private connection",((unsigned long) this)).write(Logger::Debug,connection_name);
			}

		} catch(...) {

			if(conn) {
				Logger::String{"Closing private connection due to initialization error"}.error(connection_name);
				dbus_connection_flush(conn);
				dbus_connection_close(conn);
				dbus_connection_unref(conn);
				conn = NULL;
			}

			throw;

		}

	}

	DBus::NamedBus::NamedBus(const char *connection_name, const char *address) : DBus::NamedBus::NamedBus{connection_name,NamedConnectionFactory(address)} {
	}


	DBus::NamedBus::~NamedBus() {

		dbus_connection_flush(conn);
		close();

		int fd = -1;
		if(dbus_connection_get_socket(conn,&fd)) {
			Logger::String("Closing private connection ",((unsigned long) this)," on socket '",fd,"'").write(Logger::Debug,name());
		} else {
			Logger::String("Closing private connection ",((unsigned long) this)).write(Logger::Debug,name());
		}

		dbus_connection_close(conn);
		dbus_connection_unref(conn);
		conn = NULL;
	}

 }


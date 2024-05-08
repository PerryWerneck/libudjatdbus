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

		DBusError err;
		dbus_error_init(&err);

		DBusConnection *connection = dbus_connection_open(address, &err);
		if(dbus_error_is_set(&err)) {
			Logger::String message{"Cant open '",address,"': ",err.message};
			dbus_error_free(&err);
			throw runtime_error(message);
		}

		return connection;

	}

	DBus::NamedBus::NamedBus(const char *connection_name, const char *address) : Abstract::DBus::Connection{connection_name,NamedConnectionFactory(address)} {

		if(!(address && *address)) {
			throw system_error(EINVAL,system_category(),"Invalid D-bus address");
		}

		try {

			open();
			bus_register();

			int fd = -1;
			if(dbus_connection_get_socket(conn,&fd)) {
				Logger::String("Got connection to '",address,"' on socket '",fd,"'").write(Logger::Debug,connection_name);
			} else {
				Logger::String("Got connection to '",address,"'").write(Logger::Debug,connection_name);
			}

		} catch(...) {

			if(conn) {
				Logger::String{"Closing connection to '",connection_name,"' due to initialization error"}.error("d-bus");
				dbus_connection_flush(conn);
				dbus_connection_unref(conn);
				conn = nullptr;
			}

			throw;

		}

	}

	DBus::NamedBus::~NamedBus() {

		int fd = -1;
		if(dbus_connection_get_socket(conn,&fd)) {
			Logger::String("Closing named connection on socket '",fd,"'").write(Logger::Debug,name());
		} else {
			Logger::String("Closing named connection").write(Logger::Debug,name());
		}

		close();
		dbus_connection_flush(conn);
		dbus_connection_unref(conn);
	}

 }


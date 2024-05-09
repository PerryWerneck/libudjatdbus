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
  * @brief Implements system bus connection.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <dbus/dbus.h>
 #include <udjat/tools/dbus/connection.h>

 namespace Udjat {

	static DBusConnection * SystemConnectionFactory() {

		Udjat::DBus::initialize();

		DBusError err;
		dbus_error_init(&err);

		DBusConnection * connct = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
		if(dbus_error_is_set(&err)) {
			std::string message(err.message);
			dbus_error_free(&err);
			throw std::runtime_error(message);
		}

		return connct;

	}

	DBus::SystemBus::SystemBus() : Abstract::DBus::Connection{"SysBUS",SystemConnectionFactory()} {
		bind();
		open();
	}

	DBus::SystemBus::~SystemBus() {
		close();
		unbind();
		dbus_connection_unref(conn);
	}

	DBus::SystemBus & DBus::SystemBus::getInstance() {
		static DBus::SystemBus instance;
		return instance;
	}

 }


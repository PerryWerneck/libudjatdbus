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
  * @brief Implements d-bus exception.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/dbus/exception.h>
 #include <stdexcept>
 #include <string>
 #include <udjat/tools/string.h>

 using namespace std;

 namespace Udjat {

	std::string build_message(DBusMessage *message, const char *error_text) {
		if(error_text && *error_text) {
			return error_text;
		}
		return String{dbus_message_get_interface(message),".",dbus_message_get_member(message)};
	}

	DBus::Exception::Exception(DBusMessage *message, const char *error_name, const char *error_text)
		: std::runtime_error{build_message(message,error_text)}, error_message{dbus_message_new_error(message,error_name,what())} {
	}

	DBus::Exception::~Exception() {
		dbus_message_unref(error_message);
	}

	void DBus::Exception::send(DBusConnection *connct) const noexcept {
		dbus_connection_send(connct, error_message, NULL);
	}

	void DBus::Error::verify() {
		if(dbus_error_is_set(&err)) {
			throw runtime_error(err.message);
		}
	}

 }



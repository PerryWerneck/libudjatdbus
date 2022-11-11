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
 #include <udjat/tools/logger.h>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	DBus::Message::Message(const char *destination, const char *path, const char *iface, const char *method) {
		message.value = dbus_message_new_method_call(destination, path, iface, method);
		dbus_message_iter_init_append(message.value, &message.iter);
	}

	DBus::Message::Message(const DBusError &error) {
		this->message.value = nullptr;
		this->err.valid = true;
		this->err.name = error.name;
		this->err.message = error.message;
	}

	DBus::Message::Message(DBusMessage *message) {

		if(dbus_message_get_type(message) == DBUS_MESSAGE_TYPE_ERROR) {

			err.valid = true;
			err.name = dbus_message_get_error_name(message);

			debug("Error name=",err.name);

			// Get error message.
			DBusMessageIter iter;
			dbus_message_iter_init(message, &iter);

			err.message.clear();
			if(dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_STRING) {
				DBusBasicValue value;
				dbus_message_iter_get_basic(&iter,&value);
				err.message = value.str;
				debug("Error message=",err.message);
			}


		} else {

			this->message.value = message;
			dbus_message_ref(message);
			dbus_message_iter_init(this->message.value, &this->message.iter);

		}

	}

	DBus::Message::~Message() {
		if(message.value) {
			dbus_message_unref(message.value);
		}
	}

	DBusMessageIter * DBus::Message::getIter() {
		if(err.valid) {
			throw runtime_error(err.message);
		}
		return & this->message.iter;
	}


	bool DBus::Message::next() {
		if(err.valid) {
			throw runtime_error(err.message);
		}
		return dbus_message_iter_next(&message.iter);
	}

	DBus::Message & DBus::Message::pop(Value &value) {

		if(err.valid) {
			throw runtime_error(err.message);
		}

		if(value.set(&message.iter))
			dbus_message_iter_next(&message.iter);

		return *this;
	}

 }

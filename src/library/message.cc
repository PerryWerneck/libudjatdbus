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
 #include <iostream>

 using namespace std;

 namespace Udjat {

	DBus::Message::Message(const DBusError &error) {
		this->error.valid = true;
		this->error.name = error.name;
		this->error.message = error.message;
	}

	DBus::Message::Message(DBusMessage *message) {
		this->message.value = message;
		dbus_message_ref(message);
		dbus_message_iter_init(this->message.value, &this->message.iter);
	}

	DBus::Message::~Message() {
		dbus_message_unref(message.value);
	}

	DBusMessageIter * DBus::Message::getIter() {
		if(error.valid) {
			throw runtime_error(error.message);
		}
		return & this->message.iter;
	}


	bool DBus::Message::next() {
		if(error.valid) {
			throw runtime_error(error.message);
		}
		return dbus_message_iter_next(&message.iter);
	}

	DBus::Message & DBus::Message::pop(Value &value) {

		if(error.valid) {
			throw runtime_error(error.message);
		}

		if(value.set(&message.iter))
			dbus_message_iter_next(&message.iter);

		return *this;
	}

 }

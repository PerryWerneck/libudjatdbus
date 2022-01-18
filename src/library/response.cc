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

 using namespace std;

 namespace Udjat {

	/*
	DBus::Response::Response() {

	}

	DBus::Response::~Response() {
	}

	void DBus::Response::get(DBusMessage *message) {

		DBusMessageIter iter;
		dbus_message_iter_init_append(message, &iter);

		if(value.children.empty() || value == DBUS_TYPE_ARRAY) {

			value.get(&iter);

		} else {

			for(auto child : value.children) {
				child.second->get(&iter);
			}

		}

	}

	void DBus::Response::reply(DBusConnection *connection, DBusMessage *message) {
		DBusMessage * reply = dbus_message_new_method_return(message);
		get(reply);
		dbus_connection_send(connection, reply, NULL);
	}

	bool DBus::Response::isNull() const {
		return value.isNull();
	}

	Udjat::Value & DBus::Response::reset(const Udjat::Value::Type type) {
		value.reset(type);
		return *this;
	}

	Udjat::Value & DBus::Response::operator[](const char *name) {
		return value[name];
	}

	Udjat::Value & DBus::Response::append(const Type type) {
		return value.append(type);
	}

	Udjat::Value & DBus::Response::set(const Value &value) {
		return this->value.set(value);
	}

	Udjat::Value & DBus::Response::set(const char *value, const Type type) {
		throw system_error(ENOTSUP,system_category(),"Cant set value by type");
		return *this;
	}
	*/

 }


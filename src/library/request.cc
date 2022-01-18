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

	DBus::Request::Request(DBusMessage *m, const std::string &a) : message(m),action(a) {

		this->method = dbus_message_get_member(message);
		dbus_message_ref(message);

		dbus_message_iter_init(message, &iter);

	}

	DBus::Request::~Request() {
		dbus_message_unref(message);
	}

	const std::string DBus::Request::getAction() {
#ifdef DEBUG
		cout << "Request action: '" << this->action << "'" << endl;
#endif // DEBUG
		return this->action;
	}

	int DBus::Request::pop(DBusBasicValue &value) {

		int type = dbus_message_iter_get_arg_type(&iter);
		if(type == DBUS_TYPE_INVALID) {
			throw system_error(ENODATA,system_category(),"Cant 'pop' required argument");
		}

		dbus_message_iter_get_basic(&iter,&value);

		dbus_message_iter_next(&iter);
		return type;
	}

	std::string DBus::Request::pop() {

		DBusBasicValue value;

		switch(pop(value)) {
		case DBUS_TYPE_INT16:
			return to_string(value.i16);

		case DBUS_TYPE_INT32:
			return to_string(value.i32);

		case DBUS_TYPE_INT64:
			return to_string(value.i64);

		case DBUS_TYPE_UINT16:
			return to_string(value.u16);

		case DBUS_TYPE_UINT32:
			return to_string(value.u32);

		case DBUS_TYPE_UINT64:
			return to_string(value.u64);

		case DBUS_TYPE_STRING:
			return string(value.str);

		}

		throw runtime_error("Invalid data format");

	}

	Udjat::Request & DBus::Request::pop(int &out) {

		DBusBasicValue value;

		switch(pop(value)) {
		case DBUS_TYPE_INT16:
			out = (int) value.i16;
			break;

		case DBUS_TYPE_INT32:
			out = (int) value.i32;
			break;

		case DBUS_TYPE_INT64:
			out = (int) value.i64;
			break;

		case DBUS_TYPE_STRING:
			out = stoi(value.str);
			break;

		case DBUS_TYPE_UINT16:
			out = value.i16;
			break;

		case DBUS_TYPE_UINT32:
			out = value.i32;
			break;

		case DBUS_TYPE_UINT64:
			out = value.i64;
			break;

		default:
			throw runtime_error("Invalid data format");

		}

		return *this;

	}

	Udjat::Request & DBus::Request::pop(unsigned int &out) {

		DBusBasicValue value;

		switch(pop(value)) {
		case DBUS_TYPE_INT16:
			out = (int) value.i16;
			break;

		case DBUS_TYPE_INT32:
			out = (int) value.i32;
			break;

		case DBUS_TYPE_INT64:
			out = (int) value.i64;
			break;

		case DBUS_TYPE_STRING:
			out = stoi(value.str);
			break;

		case DBUS_TYPE_UINT16:
			out = value.i16;
			break;

		case DBUS_TYPE_UINT32:
			out = value.i32;
			break;

		case DBUS_TYPE_UINT64:
			out = value.i64;
			break;

		default:
			throw runtime_error("Invalid data format");

		}

		return *this;

	}

 }


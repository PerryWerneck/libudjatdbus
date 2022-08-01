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

	DBus::Message::Message(const char *destination, const char *path, const char *iface, const char *method) {
		message.value = dbus_message_new_method_call(destination, path, iface, method);
		dbus_message_iter_init_append(message.value, &message.iter);
	}

	DBus::Message::Message(const DBusError &error) {
		this->message.value = nullptr;
		this->error.valid = true;
		this->error.name = error.name;
		this->error.message = error.message;
	}

	DBus::Message::Message(DBusMessage *message) {

		if(dbus_message_get_type(message) == DBUS_MESSAGE_TYPE_ERROR) {

			error.valid = true;
			error.name = dbus_message_get_error_name(message);

#ifdef DEBUG
			cout << "Error name=" << error.name << endl;
#endif // DEBUG

			// Get error message.
			DBusMessageIter iter;
			dbus_message_iter_init(message, &iter);

			error.message.clear();
			if(dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_STRING) {
				DBusBasicValue value;
				dbus_message_iter_get_basic(&iter,&value);
				error.message = value.str;
#ifdef DEBUG
				cout << "Error message=" << error.message << endl;
#endif // DEBUG
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

	DBus::Message & DBus::Message::push_back(const char *value) {
		if(!dbus_message_iter_append_basic(&message.iter,DBUS_TYPE_STRING,&value)) {
			throw runtime_error("Can't add value to d-bus iterator");
		}
		return *this;
	}

	DBus::Message & DBus::Message::push_back(const bool value) {

		dbus_bool_t dvalue = value;

		if(!dbus_message_iter_append_basic(&message.iter,DBUS_TYPE_BOOLEAN,&dvalue)) {
			throw runtime_error("Can't add value to d-bus iterator");
		}

		return *this;
	}

	DBus::Message & DBus::Message::push_back(const int16_t value) {

		dbus_int16_t dvalue = value;

		if(!dbus_message_iter_append_basic(&message.iter,DBUS_TYPE_INT16,&dvalue)) {
			throw runtime_error("Can't add value to d-bus iterator");
		}

		return *this;
	}

	DBus::Message & DBus::Message::push_back(const uint16_t value) {

		dbus_uint16_t dvalue = value;

		if(!dbus_message_iter_append_basic(&message.iter,DBUS_TYPE_UINT16,&dvalue)) {
			throw runtime_error("Can't add value to d-bus iterator");
		}

		return *this;
	}

	DBus::Message & DBus::Message::push_back(const int32_t value) {

		dbus_int32_t dvalue = value;

		if(!dbus_message_iter_append_basic(&message.iter,DBUS_TYPE_INT32,&dvalue)) {
			throw runtime_error("Can't add value to d-bus iterator");
		}

		return *this;
	}

	DBus::Message & DBus::Message::push_back(const uint32_t value) {

		dbus_uint32_t dvalue = value;

		if(!dbus_message_iter_append_basic(&message.iter,DBUS_TYPE_UINT32,&dvalue)) {
			throw runtime_error("Can't add value to d-bus iterator");
		}

		return *this;
	}

	DBus::Message & DBus::Message::push_back(const int64_t value) {

		dbus_int64_t dvalue = value;

		if(!dbus_message_iter_append_basic(&message.iter,DBUS_TYPE_INT64,&dvalue)) {
			throw runtime_error("Can't add value to d-bus iterator");
		}

		return *this;
	}

	DBus::Message & DBus::Message::push_back(const uint64_t value) {

		dbus_uint64_t dvalue = value;

		if(!dbus_message_iter_append_basic(&message.iter,DBUS_TYPE_UINT64,&dvalue)) {
			throw runtime_error("Can't add value to d-bus iterator");
		}

		return *this;
	}

 }

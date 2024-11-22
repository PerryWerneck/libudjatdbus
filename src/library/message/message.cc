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
 #include <udjat/defs.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/dbus/message.h>
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

	static void to_value(DBusMessageIter *iter, Udjat::Value &value) {

		DBusBasicValue dval;

		value.clear();

		switch(dbus_message_iter_get_arg_type(iter)) {
		case DBUS_TYPE_INVALID:
			throw runtime_error("Invalid d-bus value");

		case DBUS_TYPE_ARRAY:
			// TODO: Convert to Value::Array
			// Reference: https://android.googlesource.com/platform/frameworks/base/+/a45746e/core/jni/android_bluetooth_common.cpp
			throw runtime_error("Unsupported DBUS_TYPE_ARRAY value");

		case DBUS_TYPE_DICT_ENTRY:
			// TODO: Convert to Value::Object
			throw runtime_error("Unsupported DBUS_TYPE_DICT_ENTRY value");

		case DBUS_TYPE_VARIANT:
			{
				DBusMessageIter sub;
				dbus_message_iter_recurse(iter, &sub);
				to_value(&sub,value);
			}

		case DBUS_TYPE_STRING:
		case DBUS_TYPE_OBJECT_PATH:
			dbus_message_iter_get_basic(iter,&dval);
			value.set(dval.str);
			break;

		case DBUS_TYPE_BOOLEAN:
			dbus_message_iter_get_basic(iter,&dval);
			value.set((bool) dval.bool_val);
			break;

		case DBUS_TYPE_INT16:
			dbus_message_iter_get_basic(iter,&dval);
			value.set((int) dval.i16);
			break;

		case DBUS_TYPE_INT32:
			dbus_message_iter_get_basic(iter,&dval);
			value.set((int) dval.i32);
			break;

		default:
			throw runtime_error("Unexpected d-bus value");

		}

		return;

	}

	DBus::Message & DBus::Message::pop(Udjat::Value &value) {
		if(err.valid) {
			throw runtime_error(err.message);
		}
		to_value(&message.iter,value);
		return *this;
	}

	static int to_value(DBusMessageIter *iter, DBusBasicValue &value) {
		int type = dbus_message_iter_get_arg_type(iter);

		if(type == DBUS_TYPE_VARIANT) {
			DBusMessageIter sub;
			dbus_message_iter_recurse(iter, &sub);
			return to_value(&sub,value);
		}

		return type;
	}

	DBus::Message & DBus::Message::pop(std::string &value) {

		DBusBasicValue dval;
		switch(to_value(&message.iter,dval)) {
		case DBUS_TYPE_STRING:
		case DBUS_TYPE_OBJECT_PATH:
			value = dval.str;
			break;

		default:
			throw runtime_error("Unexpected d-bus value");

		}

		return *this;
	}

	DBus::Message & DBus::Message::pop(int &value) {

		DBusBasicValue dval;
		switch(to_value(&message.iter,dval)) {
		case DBUS_TYPE_STRING:
			value = atoi(dval.str);
			break;

		case DBUS_TYPE_BOOLEAN:
			value = (int) dval.bool_val;
			break;

		case DBUS_TYPE_INT16:
			value = (int) dval.i16;
			break;

		case DBUS_TYPE_INT32:
			value = (int) dval.i32;
			break;

		case DBUS_TYPE_UINT16:
			value = (int) dval.u16;
			break;

		case DBUS_TYPE_UINT32:
			value = (int) dval.u32;
			break;
			
		default:
			throw runtime_error("Unexpected d-bus value");

		}

		return *this;

	}

	DBus::Message & DBus::Message::pop(unsigned int &value) {

		DBusBasicValue dval;
		switch(to_value(&message.iter,dval)) {
		case DBUS_TYPE_STRING:
			value = (unsigned int) atoi(dval.str);
			break;

		case DBUS_TYPE_BOOLEAN:
			value = (unsigned int) dval.bool_val;
			break;

		case DBUS_TYPE_INT16:
			value = (unsigned int) dval.i16;
			break;

		case DBUS_TYPE_INT32:
			value = (unsigned int) dval.i32;
			break;

		case DBUS_TYPE_UINT16:
			value = (unsigned int) dval.u16;
			break;

		case DBUS_TYPE_UINT32:
			value = (unsigned int) dval.u32;
			break;
			
		default:
			throw runtime_error("Unexpected d-bus value");

		}

		return *this;

	}

	DBus::Message & DBus::Message::pop(bool &value) {

		DBusBasicValue dval;
		switch(to_value(&message.iter,dval)) {
		case DBUS_TYPE_STRING:
			value = String{dval.str}.as_bool();
			break;

		case DBUS_TYPE_BOOLEAN:
			value = dval.bool_val;
			break;

		case DBUS_TYPE_INT16:
			value = dval.i16;
			break;

		case DBUS_TYPE_INT32:
			value = dval.i32;
			break;

		case DBUS_TYPE_UINT16:
			value = dval.u16;
			break;

		case DBUS_TYPE_UINT32:
			value = dval.u32;
			break;
			
		default:
			throw runtime_error("Unexpected d-bus value");

		}

		return *this;

	}

	DBus::Message & DBus::Message::pop(double &value) {

		DBusBasicValue dval;
		switch(to_value(&message.iter,dval)) {
		case DBUS_TYPE_STRING:
			value = atof(dval.str);
			break;

		case DBUS_TYPE_DOUBLE:
			value = dval.dbl;
			break;

		default:
			throw runtime_error("Unexpected d-bus value");

		}

		return *this;
	}

	DBus::Message & DBus::Message::push_back(const bool value) {
		dbus_bool_t dval = value;
		dbus_message_iter_append_basic(&message.iter, DBUS_TYPE_BOOLEAN, &dval);
		return *this;
	}

	DBus::Message & DBus::Message::push_back(const int value) {
		dbus_int32_t dval = value;
		dbus_message_iter_append_basic(&message.iter, DBUS_TYPE_INT32, &dval);
		return *this;
	}

	DBus::Message & DBus::Message::push_back(const unsigned int value) {
		dbus_uint32_t dval = value;
		dbus_message_iter_append_basic(&message.iter, DBUS_TYPE_UINT32, &dval);
		return *this;
	}

	DBus::Message & DBus::Message::push_back(const double value) {
		dbus_message_iter_append_basic(&message.iter, DBUS_TYPE_DOUBLE, &value);
		return *this;
	}

 }

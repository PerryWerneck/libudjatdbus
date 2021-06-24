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

 #include <udjat/dbus.h>
 #include <cstring>

 namespace Udjat {

	DBus::Value::Value() {
		type = DBUS_TYPE_BOOLEAN;
		value.bool_val = 0;
	}

	DBus::Value::~Value() {
		reset(Value::Type::Undefined);
	}

	Udjat::Value & DBus::Value::reset(const Value::Type unused) {

		if(type == DBUS_TYPE_STRING && value.str) {
			free(value.str);
			value.str = NULL;
		}

		type = DBUS_TYPE_BOOLEAN;
		value.bool_val = 0;

		return *this;
	}

	Udjat::Value & DBus::Value::set(const char *value, const Type type) {
		reset(Value::Type::Undefined);
		this->type = DBUS_TYPE_STRING;
		this->value.str = strdup(value);
		return *this;
	}

	Udjat::Value & DBus::Value::set(const short value) {
		reset(Value::Type::Undefined);
		this->type = DBUS_TYPE_INT16;
		this->value.i16 = value;
		return *this;
	}

	Udjat::Value & DBus::Value::set(const unsigned short value) {
		reset(Value::Type::Undefined);
		this->type = DBUS_TYPE_UINT16;
		this->value.u16 = value;
		return *this;
	}

	Udjat::Value & DBus::Value::set(const int value) {
		reset(Value::Type::Undefined);
		this->type = DBUS_TYPE_INT32;
		this->value.i32 = value;
		return *this;
	}

	Udjat::Value & DBus::Value::set(const unsigned int value) {
		reset(Value::Type::Undefined);
		this->type = DBUS_TYPE_UINT32;
		this->value.u32 = value;
		return *this;
	}

	Udjat::Value & DBus::Value::set(const long value) {
		reset(Value::Type::Undefined);
		this->type = DBUS_TYPE_INT64;
		this->value.i64 = value;
		return *this;
	}

	Udjat::Value & DBus::Value::set(const unsigned long value) {
		reset(Value::Type::Undefined);
		this->type = DBUS_TYPE_UINT64;
		this->value.u64 = value;
		return *this;
	}

	Udjat::Value & DBus::Value::set(const TimeStamp value) {
		reset(Value::Type::Undefined);
		return set(value.to_string().c_str());
	}

	Udjat::Value & DBus::Value::set(const bool value) {
		reset(Value::Type::Undefined);
		this->type = DBUS_TYPE_BOOLEAN;
		this->value.bool_val = value;
		return *this;
	}

	Udjat::Value & DBus::Value::set(const float value) {
		reset(Value::Type::Undefined);
		this->type = DBUS_TYPE_DOUBLE;
		this->value.dbl = value;
		return *this;
	}

	Udjat::Value & DBus::Value::set(const double value) {
		reset(Value::Type::Undefined);
		this->type = DBUS_TYPE_DOUBLE;
		this->value.dbl = value;
		return *this;
	}

 }


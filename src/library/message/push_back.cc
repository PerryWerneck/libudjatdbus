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

 /*
 #include <config.h>
 #include <udjat/tools/dbus.h>
 #include <udjat/tools/logger.h>
 #include <iostream>

 using namespace std;

 namespace Udjat {

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

	DBus::Message & DBus::Message::push_back(const DBus::Value &value) {
		value.get(&message.iter);
		return *this;

	}

	DBus::Message & DBus::Message::push_back(const std::vector<std::string> &elements) {

		DBusMessageIter iter;

		if(!dbus_message_iter_open_container(&message.iter, DBUS_TYPE_ARRAY, "s", &iter)) {
			throw runtime_error("Error opening D-Bus container");
		}

		for(auto it = elements.begin(); it != elements.end(); it++) {

			const char *value = it->c_str();

			if(!dbus_message_iter_append_basic(&iter,DBUS_TYPE_STRING,&value)) {
				throw runtime_error("Can't add value to d-bus container");
			}

		}

		if(!dbus_message_iter_close_container(&message.iter, &iter)) {
			throw runtime_error("Error closing D-Bus container");
		}

		return *this;
	}

	std::ostream & DBus::Message::info() const {
		return std::cout << name << "\t";
	}

	std::ostream & DBus::Message::warning() const {
		return std::clog << name << "\t";
	}

	std::ostream & DBus::Message::error() const {
		return std::cerr << name << "\t";
	}

	std::ostream & DBus::Message::trace() const {
		return Logger::trace() << name << "\t";
	}

 }
 */

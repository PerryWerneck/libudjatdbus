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
 #include "private.h"
 #include <udjat/tools/logger.h>

 using namespace std;

 namespace Udjat {

	DBus::Signal::Signal(const char *iface, const char *member, const char *path) {

		debug("iface=",iface);
		debug("member=",member);
		debug("path=",path);

		if(!*iface) {
			throw system_error(EINVAL,system_category(),"Empty D-Bus interface name");
		}

		if(!*member) {
			throw system_error(EINVAL,system_category(),"Empty D-Bus member name");
		}

		if(!*path) {
			throw system_error(EINVAL,system_category(),"Empty D-Bus path");
		}

		message = dbus_message_new_signal(path,iface,member);
		dbus_message_iter_init_append(message, &iter);

	}

	DBus::Signal::~Signal() {

		dbus_message_unref(message);

	}

	void DBus::Signal::system() {
		send(Connection::getSystemInstance());
	}

	void DBus::Signal::session() {
		send(Connection::getSessionInstance());
	}

	void DBus::Signal::starter() {
		send(Connection::getStarterInstance());
	}

	void DBus::Signal::send() {
		send(Connection::getInstance());
	}

	void DBus::Signal::send(Connection &connection) {
		if(!connection.send(message)) {
			throw runtime_error("Cant send d-bus signal");
		}
	}

	DBus::Signal & DBus::Signal::push_back(const char *value) {
		if(!dbus_message_iter_append_basic(&iter,DBUS_TYPE_STRING,&value)) {
			throw runtime_error("Can't add value to d-bus iterator");
		}
		return *this;
	}

	DBus::Signal & DBus::Signal::push_back(const bool value) {

		dbus_bool_t dvalue = value;

		if(!dbus_message_iter_append_basic(&iter,DBUS_TYPE_BOOLEAN,&dvalue)) {
			throw runtime_error("Can't add value to d-bus iterator");
		}

		return *this;
	}

	DBus::Signal & DBus::Signal::push_back(const int16_t value) {

		dbus_int16_t dvalue = value;

		if(!dbus_message_iter_append_basic(&iter,DBUS_TYPE_INT16,&dvalue)) {
			throw runtime_error("Can't add value to d-bus iterator");
		}

		return *this;
	}

	DBus::Signal & DBus::Signal::push_back(const uint16_t value) {

		dbus_uint16_t dvalue = value;

		if(!dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT16,&dvalue)) {
			throw runtime_error("Can't add value to d-bus iterator");
		}

		return *this;
	}

	DBus::Signal & DBus::Signal::push_back(const int32_t value) {

		dbus_int32_t dvalue = value;

		if(!dbus_message_iter_append_basic(&iter,DBUS_TYPE_INT32,&dvalue)) {
			throw runtime_error("Can't add value to d-bus iterator");
		}

		return *this;
	}

	DBus::Signal & DBus::Signal::push_back(const uint32_t value) {

		dbus_uint32_t dvalue = value;

		if(!dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT32,&dvalue)) {
			throw runtime_error("Can't add value to d-bus iterator");
		}

		return *this;
	}

	DBus::Signal & DBus::Signal::push_back(const int64_t value) {

		dbus_int64_t dvalue = value;

		if(!dbus_message_iter_append_basic(&iter,DBUS_TYPE_INT64,&dvalue)) {
			throw runtime_error("Can't add value to d-bus iterator");
		}

		return *this;
	}

	DBus::Signal & DBus::Signal::push_back(const uint64_t value) {

		dbus_uint64_t dvalue = value;

		if(!dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT64,&dvalue)) {
			throw runtime_error("Can't add value to d-bus iterator");
		}

		return *this;
	}

 }
 */

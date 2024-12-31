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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/abstract/object.h>
 #include <udjat/tools/dbus/connection.h>
 #include <udjat/alert.h>
 #include <udjat/alert/d-bus.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/string.h>
 #include <dbus/dbus.h>
 #include <stdexcept>

 using namespace std;

 namespace Udjat {

	DBus::Alert::Alert(const XML::Node &node) : Udjat::Alert{node}, DBus::Emitter{node} {
	}

	DBus::Alert::~Alert() {
	}

	void DBus::Alert::reset(time_t next) noexcept {
		if(!next) {
			Emitter::clear();
		}
		super::reset(next);
	}

	bool DBus::Alert::activate() noexcept {

		if(active()) {
			return false;
		}

		prepare();
		return ::Udjat::Alert::activate();
	}

	bool DBus::Alert::activate(const Abstract::Object &object) noexcept {

		if(active()) {
			return false;
		}

		prepare(object);
		return ::Udjat::Alert::activate();
	}

	int DBus::Alert::emit() {
		Emitter::send();
		return 0;
	}

 }

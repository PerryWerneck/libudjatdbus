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
 #include <udjat/alert.h>
 #include <udjat/alert/d-bus.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/string.h>
 #include <dbus/dbus.h>
 #include <stdexcept>
 #include <udjat/tools/xml.h>

 using namespace std;

 namespace Udjat {

	DBus::Alert::Factory::Factory(const char *name) : Udjat::Alert::Factory{name} {
	}

	DBus::Alert::Factory::~Factory() {		
	}

	std::shared_ptr<Udjat::Alert> DBus::Alert::Factory::AlertFactory(const Abstract::Object &, const XML::Node &node) const {
		return make_shared<DBus::Alert>(node);
	}

	DBus::Alert::Alert(const XML::Node &node) : Udjat::Alert{node}, Udjat::DBus::Action{node} {
	}

	DBus::Alert::~Alert() {
	}

	bool DBus::Alert::activate() noexcept {
		Abstract::Object empty;
		return activate(empty);
	}

	bool DBus::Alert::activate(const Abstract::Object &object) noexcept {

		try {

			DBus::Action::call(object);

		} catch(const std::exception &e) {
			
			failed(e.what());
			return -1;

		}
		return false;

	}

	int DBus::Alert::emit() {

		try {

			DBus::Action::call(true);

		} catch(const system_error &e) {
			
			failed(e.what());
			return e.code().value();

		} catch(const std::exception &e) {
			
			failed(e.what());
			return -1;

		}

		return 0;
	}

 }

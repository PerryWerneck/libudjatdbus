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

 /**
  * @brief Implement D-Bus module & Interface factory.
  */

 // References:
 //
 // https://github.com/fbuihuu/samples-dbus/blob/master/dbus-server.c
 //

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/module/abstract.h>
 #include <udjat/module/dbus.h>
 #include <udjat/tools/interface.h>
 #include <udjat/tools/dbus/connection.h>
 #include <dbus/dbus-protocol.h>
 #include <udjat/tools/string.h>
 #include <vector>
 
 using namespace std;

 namespace Udjat {

 	static const Udjat::ModuleInfo moduleinfo { "D-Bus" STRINGIZE_VALUE_OF(DBUS_MAJOR_PROTOCOL_VERSION) " module" };

	Udjat::DBus::Module::Module() : Udjat::Module{"d-bus",moduleinfo}, Udjat::Interface::Factory{"d-bus"} {
	}

	Udjat::DBus::Module::~Module() {
	}

	Udjat::Interface & Udjat::DBus::Module::InterfaceFactory(const XML::Node &node) {
		return interfaces.emplace_back(node);
	}

	static const char *NameFactory(const XML::Node &node) {
		String name{node,"name"};
		if(name.empty()) {
			throw runtime_error("Required attribute name is empty or not found");
		}

		if(name[0] == '.') {
			name = String{}
		}

	}

	Udjat::DBus::Module::Interface::Interface(const XML::Node &n) 
		: Udjat::Interface{n}, Abstract::DBus::Interface{n},
			name{NameFactory(n)}, node{String{n,"node",""}.as_quark()} {

	}

	Udjat::DBus::Module::Interface::~Interface() {
	}

 }

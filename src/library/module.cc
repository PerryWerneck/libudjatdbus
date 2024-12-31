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
 #include <udjat/tools/dbus/service.h>
 #include <udjat/alert/d-bus.h>
 #include <dbus/dbus-protocol.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/application.h>
 #include <vector>
 
 using namespace std;

 namespace Udjat {

 	static const Udjat::ModuleInfo moduleinfo { "dbus" STRINGIZE_VALUE_OF(DBUS_MAJOR_PROTOCOL_VERSION) " module" };

	DBus::Service::Service() 
		: DBus::Service::Service{moduleinfo,"dbus",String{PRODUCT_ID,".",Application::Name().c_str()}.as_quark()} {
	}

	DBus::Module::Module() : Udjat::Module{"dbus",moduleinfo} {
		DBus::initialize();
	}

	DBus::Module::~Module() {
	}

 }

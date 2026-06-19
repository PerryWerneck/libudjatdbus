/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tools/loader.h>

 #include <udjat/module/dbus.h>
 #include <udjat/tools/dbus/service.h>
 #include <udjat/tools/dbus/connection.h>
 
 using namespace Udjat;

 int main(const int argc, const char **argv) {
	
	return loader(argc,argv, [](Application &app) -> int {

		debug("Initializing...");
		
		// class Module : public DBus::Module, public DBus::Service {
		// public:
		// 	Module()
		// 		: DBus::Module{},
		// 			DBus::Service{
		// 				DBus::Connection::getInstance(DBUS_BUS_STARTER),
		// 				"dbus",
		// 				String{PRODUCT_DOMAIN,".",Application::Name().c_str()}.as_quark()
		// 			} { 
					
		// 		autoclean();
		// 	}

		// 	virtual ~Module() {
		// 	}

		// };

		// new Module();

		return 0;

	});

 }

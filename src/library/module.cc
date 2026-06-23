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
 #include <udjat/module.h>
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

	Udjat::Module * DBus::Module::Factory(const Udjat::Properties &props) {

		/// @brief busname.
		String srvname{props["dbus-service-name"]};
		
		if(srvname.empty()) {
			srvname = props["service-name"];
		}

		if(srvname.empty()) {
			srvname = String{PRODUCT_DOMAIN,""};
		}

		if(srvname.empty() && props.get("enable-service",false)) {
			srvname = String{PRODUCT_DOMAIN,".",Application::Name().c_str()};
		}

		if(srvname.empty()) {
			// No service name, build a clean module.
			auto module = new DBus::Module();
			module->autoclean();
			return module;
		}

		/// @brief Service name.
		String name{props.get("name","dbus")};

		Logger::String{"Initializing d-bus service '",srvname.c_str(),"'"}.trace(name.c_str());

		class Module : public DBus::Module, public DBus::Service {
		public:
			Module(const Udjat::Properties &props, const char *name, const char *srvname)
				: DBus::Module{},
					DBus::Service{
						(DBusConnection *) DBus::Connection::getInstance(props),
						name,
						srvname
					} { 
						autoclean();
					}

			virtual ~Module() {
			}

		};

		return new Module(props,name.as_quark(),srvname.as_quark());

	}

 	DBus::Service::Service() 
		: DBus::Service::Service{"dbus",String{PRODUCT_DOMAIN,".",Application::Name().c_str()}.as_quark()} {
	}

	DBus::Module::Module() : Udjat::Module{"dbus","dbus " STRINGIZE_VALUE_OF(DBUS_MAJOR_PROTOCOL_VERSION) " module"} {
		DBus::initialize();
	}

	DBus::Module::~Module() {
	}

 }

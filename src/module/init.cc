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
 #include <udjat/tools/dbus/connection.h>
 #include <udjat/module/abstract.h>
 #include <udjat/tools/xml.h>
 #include <udjat/module/dbus.h>
 #include <udjat/tools/dbus/service.h>
 #include <udjat/tools/dbus/connection.h>
 #include <udjat/tools/application.h>

 using namespace Udjat;
 
 Udjat::Module * udjat_module_init() {

	class Module : public DBus::Module, private DBus::Service {
	public:
		Module() = default;
		virtual ~Module() {
		}

	};

	return new Module();
 }

 Udjat::Module * udjat_module_init_from_xml(const XML::Node &node) {

	/// @brief busname.
	String srvname{node,"dbus-service-name",""};
	if(srvname.empty() && node.attribute("enable-service").as_bool(false)) {
		srvname = String{PRODUCT_ID,".",Application::Name().c_str()};
	}

	if(srvname.empty()) {
		// No service name, just start module.
		return new DBus::Module();
	}

	/// @brief Service name.
	String name{node,"name","dbus"};

	Logger::String{"Initializing d-bus service '",srvname.c_str(),"'"}.trace(name.c_str());

	class Module : public DBus::Module, public DBus::Service {
	public:
		Module(const XML::Node &node, const char *name, const char *srvname)
			: DBus::Module{},
				DBus::Service{
					(const ModuleInfo &) *this,
					(DBusConnection *) DBus::Connection::getInstance(node),
					name,
					srvname
				} { }

		virtual ~Module() {
		}

	};

	return new Module(node,name.as_quark(),srvname.as_quark());

 }

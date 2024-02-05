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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/module.h>
 #include <udjat/moduleinfo.h>
 #include <udjat/factory.h>
 #include <dbus/dbus-protocol.h>
 #include <udjat/alert/d-bus.h>
 #include <memory>

 using namespace std;
 using namespace Udjat;

 static const Udjat::ModuleInfo moduleinfo { "D-Bus" STRINGIZE_VALUE_OF(DBUS_MAJOR_PROTOCOL_VERSION) " module" };

 /// @brief Register udjat module.
 Udjat::Module * udjat_module_init() {

	class Module : public Udjat::Module, Udjat::Factory {
	public:

		Module() : Udjat::Module("d-bus",moduleinfo), Udjat::Factory("d-bus",moduleinfo) {
		};

		virtual ~Module() {
		};

		std::shared_ptr<Abstract::Alert> AlertFactory(const Abstract::Object &parent, const pugi::xml_node &node) const override {
			return make_shared<Udjat::DBus::Alert>(parent,node);
		}

	};

	return new Module();
 }



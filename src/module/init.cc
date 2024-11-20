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
 #include <udjat/version.h>
 #include <udjat/module.h>
 #include <udjat/moduleinfo.h>
 #include <udjat/factory.h>
 #include <udjat/alert/d-bus.h>
 
 #include <memory>
 #include <dbus/dbus-protocol.h>
 
 #include <udjat/tools/xml.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/dbus/connection.h>
 #include <udjat/worker.h>
 
 using namespace std;
 using namespace Udjat;

 static const Udjat::ModuleInfo moduleinfo { "D-Bus" STRINGIZE_VALUE_OF(DBUS_MAJOR_PROTOCOL_VERSION) " module" };

 class Module : public Udjat::Module, Udjat::Factory {
 private:

#ifdef DEBUG
	DBusBusType bustype = DBUS_BUS_STARTER;
	const char *service_name = "br.eti.werneck.udjat";
#else
	DBusBusType bustype = (DBusBusType) -1;
	const char *service_name = nullptr;
#endif // DEBUG

 public:

	Module() : Udjat::Module("d-bus",moduleinfo), Udjat::Factory("d-bus",moduleinfo) {
	};

	Module(const XML::Node &node) : Module() {

		static DBusBusType types[] = {
			DBUS_BUS_SESSION,	///< @brief The login session bus.
			DBUS_BUS_SYSTEM,	///< @brief The systemwide bus.
			DBUS_BUS_STARTER	///< @brief The bus that started us, if any.
		};

		size_t type = String(node,"bus-name",(node.attribute("service").as_bool() ? "starter" : "none")).select("session","system","starter","none",NULL);

		if(type >= (sizeof(types)/sizeof(types[0]))) {
			Logger::String{"Attribute 'bus-name' is empty or invalid, no listener will be activated"}.trace("d-bus");
			return;
		}

		//
		// Get service name
		//
		String srvname{node,"dbus-service-name",""};

		if(srvname.empty()) {
			srvname = ".";
			srvname += Application::Name{};
		}

		if(srvname[0] == '.') {
			Config::Value<string> prefix{"dbus","service-prefix","br.eti.werneck"};
			srvname = prefix + srvname.c_str();
		}

		this->service_name = srvname.as_quark();

		Logger::String{"Service name set to ",this->service_name}.info("d-bus");

	}

	virtual ~Module() {
	};

	std::shared_ptr<Abstract::Alert> AlertFactory(const Abstract::Object &parent, const pugi::xml_node &node) const override {
		return make_shared<Udjat::DBus::Alert>(parent,node);
	}

 };

 /// @brief Register udjat module.
 Udjat::Module * udjat_module_init() {
	Udjat::DBus::initialize();
	return new ::Module();
 }

 Udjat::Module * udjat_module_init_from_xml(const pugi::xml_node &node) {
	return new ::Module(node);
 }


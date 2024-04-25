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
 #include <udjat/module/abstract.h>
 #include <udjat/moduleinfo.h>
 #include <udjat/factory.h>
 #include <dbus/dbus-protocol.h>
 #include <udjat/alert/d-bus.h>
 #include <memory>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/dbus/connection.h>

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


	void set(std::shared_ptr<Abstract::Agent> agent) noexcept override {

		if(bustype ==(DBusBusType) -1 || !(service_name && *service_name)) {
			return;
		}

		debug("---------------------------------------------------------> Creating service");

		// D-Bus service.
		class Service : private Abstract::DBus::Connection, public Udjat::NamedObject {
		private:
			const char *service_name;

		public:
			Service(DBusBusType bustype, const char *sname) : Abstract::DBus::Connection{"d-bus",SharedConnectionFactory(bustype)}, Udjat::NamedObject{"d-bus"}, service_name{sname} {
				Logger::String{"Starting ",service_name}.info("d-bus");
				request_name(service_name);
			}

			virtual ~Service() {
				Logger::String{"Stopping ",service_name}.info("d-bus");
			}

		};

		// New root agent, create d-bus service and add to it.
		try {

			agent->push_back(make_shared<Service>(bustype,service_name));

		} catch(const std::exception &e) {

			Logger::String{"Error '",e.what(),"' starting d-bus service"}.error("d-bus");

		} catch(...) {


			Logger::String{"Unexpected error starting d-bus service"}.error("d-bus");

		}


	}

 };

 /// @brief Register udjat module.
 Udjat::Module * udjat_module_init() {
	return new ::Module();
 }

 Udjat::Module * udjat_module_init_from_xml(const pugi::xml_node &node) {
	return new ::Module(node);
 }


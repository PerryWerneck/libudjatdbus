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

 #include "private.h"
 #include <udjat/dbus.h>
 #include <udjat/factory.h>

 using namespace Udjat;

 const Udjat::ModuleInfo DBus::moduleinfo{
	PACKAGE_NAME,								// The module name.
	"UDJat D-Bus module", 						// The module description.
	PACKAGE_VERSION, 							// The module version.
	PACKAGE_URL, 								// The package URL.
	PACKAGE_BUGREPORT 							// The bug report address.
 };

 /// @brief Register udjat module.
 Udjat::Module * udjat_module_init() {

	/// @brief Proxy for libudjat workers.
	class Proxy : public DBus::Worker {
	public:
		Proxy() {
			this->interface = "br.eti.werneck." STRINGIZE_VALUE_OF(PRODUCT_NAME) ".";
		}

		bool equal(DBusMessage *message) override {

			if(dbus_message_get_type(message) != DBUS_MESSAGE_TYPE_METHOD_CALL) {
				return false;
			}

			if(strncasecmp(dbus_message_get_interface(message),interface,strlen(interface))) {
				return false;
			}

			return true;
		}

		/// @brief Execute request.
		bool work(DBus::Request &request, DBus::Response &response) override {

			string name{request.getInterface() + strlen(interface)};

			size_t len = name.find('.');
			if(len != string::npos && len > 0) {
				name.resize(len);
			}

			const Udjat::Worker * worker = Udjat::Worker::find(name.c_str());
			if(!worker)
				return false;

			if(request == "get") {

				if(!worker->work(request,response)) {
					throw runtime_error("Method not allowed");
				}

			} else if(request == "info") {

				worker->getModuleInfo().get(response);

			} else {

				return false;

			}

			return true;
		}

	};

	class Controller : public Udjat::Module, Udjat::Factory {
	private:

		/// @brief Proxy for libudjat workers.
		Proxy * proxy;

	public:

		Controller() : Udjat::Module("d-bus",&DBus::moduleinfo), proxy(new Proxy()), Udjat::Factory("dbus-signal",&DBus::moduleinfo) {

			DBus::Connection &connection = DBus::Connection::getInstance();

			connection.request("br.eti.werneck." STRINGIZE_VALUE_OF(PRODUCT_NAME));
			connection.insert(proxy);

		};

		~Controller() {

			DBus::Connection &connection = DBus::Connection::getInstance();

			connection.remove(proxy);
			delete proxy;
		};

		void parse(Abstract::Agent &parent, const pugi::xml_node &node) const override {
			parent.push_back(make_shared<DBus::Signal>(node));
		}

	};

	return new Controller();
 }



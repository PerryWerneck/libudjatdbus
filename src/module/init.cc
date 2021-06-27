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

 namespace Udjat {

	/// @brief Proxy for libudjat workers.
	class Proxy : public DBus::Worker {
	private:
		static constexpr const char *interface = "br.eti.werneck." STRINGIZE_VALUE_OF(PRODUCT_NAME) ".";

	public:
		Proxy() = default;

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

			const char * name = request.getInterface() + strlen(interface);

			const Udjat::Worker * worker = Udjat::Worker::find(name);
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

	static const Udjat::ModuleInfo moduleinfo{
		PACKAGE_NAME,								// The module name.
		"D-Bus module exporter", 					// The module description.
		PACKAGE_VERSION, 							// The module version.
		PACKAGE_URL, 								// The package URL.
		PACKAGE_BUGREPORT 							// The bug report address.
	 };

 	DBus::Controller::Controller() : Udjat::Module("d-bus",&moduleinfo), proxy(new Proxy()) {

		Connection &connection = Connection::getInstance();

 		connection.request("br.eti.werneck." STRINGIZE_VALUE_OF(PRODUCT_NAME));
 		connection.insert(proxy);

 	};

	DBus::Controller::~Controller() {

		Connection &connection = Connection::getInstance();

		connection.remove(proxy);
		delete proxy;
 	};

 }


 /// @brief Register udjat module.
 Udjat::Module * udjat_module_init() {
	return new Udjat::DBus::Controller();
 }



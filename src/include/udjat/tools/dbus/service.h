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
  * @brief Declare D-Bus service.
  */

 #pragma once
 
 #include <udjat/defs.h>
 #include <udjat/module/info.h>
 #include <dbus/dbus.h>
 #include <udjat/tools/dbus/defs.h>
 #include <udjat/tools/interface.h>
 #include <udjat/tools/service.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/string.h>
 #include <vector>
 #include <sstream>  

 namespace Udjat {

	namespace DBus {

		class UDJAT_API Service : public Udjat::Service, protected Udjat::Interface::Factory {
		private:

			/// @brief Connection to D-Bus.
			DBusConnection * conn = nullptr;

			/// @brief Service name on d-bus
			const char *dest = nullptr;

			/// @brief Message filter method.
			static DBusHandlerResult on_message(DBusConnection *, DBusMessage *, DBus::Service *) noexcept;

		protected:

			/// @brief handle signals.
			/// @return Status.
			/// @retval true The signal was handled.
			/// @retval false The signal was not handled.
			bool on_signal(Udjat::DBus::Message &request);

			Udjat::Interface & InterfaceFactory(const XML::Node &node) override;

			class Interface : public Udjat::Interface, public std::vector<Udjat::Interface::Handler> {
			private:
				const char *intfname;

			public:
				Interface(const XML::Node &node, const char *intfname);
				virtual ~Interface();

				DBusHandlerResult on_message(DBusConnection *connct, DBusMessage *message, DBus::Service &service);

				Udjat::Interface::Handler & push_back(const XML::Node &node) override;

				void introspect(std::stringstream &xmldata) const;
				bool push_back(const XML::Node &node, std::shared_ptr<Action> action) override;

				inline const char * interface() const noexcept {
					return intfname;
				}

			};

			std::vector<Interface> interfaces;

		public:

			Service();
			Service(const ModuleInfo &module, const char *name, const char *destination);
			Service(const ModuleInfo &module, DBusConnection * conn, const char *name, const char *destination);

			/// @brief Find interface.
			/// @param name The name of requested interface.
			/// @return The interface, exception if not found.
			Interface & interface(const char *name);

			inline const char *name() const noexcept {
				return service_name;
			}

			static const char * ServiceNameFactory(const XML::Node &node);

			virtual ~Service();

			void start() override;
			void stop() override;

		};

	}

 }

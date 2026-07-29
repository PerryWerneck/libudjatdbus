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
 #include <dbus/dbus.h>
 #include <udjat/tools/dbus/defs.h>
 #include <udjat/tools/service.h>
 #include <udjat/tools/properties.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/interface.h>
 #include <vector>
 #include <sstream>  

 namespace Udjat {

	namespace DBus {

		class UDJAT_API Service : public Udjat::Service {
		private:

			/// @brief Connection to D-Bus.
			DBusConnection * conn = nullptr;

			/// @brief Service name on d-bus
			const char *dest = nullptr;

			/// @brief Message filter method.
			static DBusHandlerResult on_message(DBusConnection *, DBusMessage *, DBus::Service *) noexcept;

			/// @brief Process message, return response.
			/// @param interface The interface to process the message.
			/// @param name The full interface name.
			/// @param message The message to process.
			/// @return The response message.
			DBusMessage * process(const Udjat::Interface &interface, const char *name, DBusMessage *message);

		protected:

			/// @brief handle signals.
			/// @return Status.
			/// @retval true The signal was handled.
			/// @retval false The signal was not handled.
			bool on_signal(Udjat::DBus::Message &request);

		public:

			Service();
			Service(const char *name, const char *destination);
			Service(DBusConnection * conn, const char *name, const char *destination);

			inline const char *name() const noexcept {
				return service_name;
			}

			static const char * ServiceNameFactory(const Properties &props);

			virtual ~Service();

			void start() override;
			void stop() override;

		};

	}

 }

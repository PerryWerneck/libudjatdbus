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
  * @brief Declare D-Bus connection.
  */

 #pragma once

 #include <udjat/defs.h>
 #include <udjat/module/info.h>
 #include <dbus/dbus.h>
 #include <udjat/tools/dbus/defs.h>
 #include <udjat/tools/service.h>
 #include <udjat/tools/xml.h>

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

			Service(const ModuleInfo &module, DBusConnection * conn, const char *name, const char *destination);

		protected:

			/// @brief handle signals.
			/// @return Status.
			/// @retval true The signal was handled.
			/// @retval false The signal was not handled.
			virtual bool on_signal(Udjat::DBus::Message &request);

			/// @brief handle methods.
			/// @return Status.
			/// @retval true The method was handled.
			/// @retval false The method was not handled.
			virtual bool on_method(Udjat::DBus::Message &request, Udjat::Value &response);

		public:

			Service(const ModuleInfo &module, const char *name, const char *destination);
			Service(const ModuleInfo &module, const XML::Node &node);
			virtual ~Service();

			void start() override;
			void stop() override;

		};

	}


 }

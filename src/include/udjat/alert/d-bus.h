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
  * @brief Declare d-bus alerts.
  */

 #pragma once

 #include <udjat/defs.h>
 #include <udjat/alert/abstract.h>
 #include <dbus/dbus-shared.h>

 namespace Udjat {

	namespace DBus {

		class UDJAT_API Alert : public Udjat::Abstract::Alert {
		public:

			/// @brief D-Bus message argument.
			struct Argument {
				int type;			///< @brief D-Bus data type.
				String value;		///< @brief Argument value.

				/// @brief Construct D-Bus argument from XML node.
				/// @param parent Parent object.
				/// @param group Group name.
				/// @param node XML node for argument properties.
				Argument(const Abstract::Object &parent, const char *group, const pugi::xml_node &node);
			};

		private:

			/// @brief The bus type for alert.
			DBusBusType bustype = DBUS_BUS_SESSION;

			/// @brief The path to the object emitting the signal.
			const char *path = nullptr;

			/// @brief The interface the signal is emitted from.
			const char *iface = nullptr;

			/// @brief Name of the signal.
			const char *member = nullptr;

			/// @brief D-Bus message arguments.
			std::vector<Argument> arguments;

		public:
			Alert(const Abstract::Object &parent, const pugi::xml_node &node);
			virtual ~Alert();

			std::shared_ptr<Udjat::Alert::Activation> ActivationFactory() const override;

			inline const std::vector<Argument> args() const noexcept {
				return arguments;
			}

			inline DBusBusType bus() const noexcept {
				return bustype;
			}

		};

	}

 }

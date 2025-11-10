/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2025 Perry Werneck <perry.werneck@gmail.com>
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

 #pragma once

 #include <dbus/dbus.h>
 #include <udjat/defs.h>
 #include <udjat/tools/actions/abstract.h>
 #include <udjat/tools/request.h>
 #include <udjat/tools/response.h>
 #include <udjat/tools/xml.h>
 #include <vector>

 namespace Udjat {
 
 	namespace DBus {
 
		typedef int DBusType;

		class UDJAT_API Action : public Udjat::Action {
		public:

			class Factory : public Udjat::Action::Factory {
			public:
				Factory(const char *name = "dbus") : Udjat::Action::Factory{name} {
				}

				std::shared_ptr<Udjat::Action> ActionFactory(const XML::Node &node) const override;

			};

			Action(const XML::Node &node);
			Action(int message_type, DBusBusType bustype, const char *destination, const char *path, const char *interface, const char *member);

			~Action() override;

			int call(Udjat::Request &request, Udjat::Response &response, bool except) override;

			/// @brief Call action using already set parameters.
			/// @param except if true will launch exceptions on errors.
			/// @return 0 if ok, error code otherwise.
			int call(bool except = true) override;

			/// Update arguments from object.
			/// @param object The object with new argument values.
			void set(const Udjat::Abstract::Object &object);

			/// @brief D-Bus message arguments.
			struct Argument {
				const char *name;				///< @brief Argument name.
				const char *tmplt = nullptr;	///< @brief Argument template (for string parsing).
				const DBusType type;			///< @brief D-Bus data type.

				Argument(const char *n, const DBusType d, const char *t = nullptr)
					: name{n}, tmplt{t}, type{d} {
				}

				Argument(const XML::Node &node);
			};
			
		private:

			/// @brief The d-bus message type.
			int message_type = DBUS_MESSAGE_TYPE_SIGNAL;

			/// @brief The bus type for alert.
			DBusBusType bustype = DBUS_BUS_STARTER;

			/// @brief Destination.
			const char *destination = nullptr;

			/// @brief The path to the object emitting the alert.
			const char *path = nullptr;

			/// @brief The interface the alert is emitted from.
			const char *iface = nullptr;

			/// @brief The alert member.
			const char *member = nullptr;

			/// @brief Load argument values from request.
			/// @param request The request with argument values.
			void load(const Udjat::Request &request);

			/// @brief Load argument values from object.
			/// @param object The object with argument values.
			void load(const Udjat::Abstract::Object &object);

			std::vector<Argument> arguments;

			/// @brief The argument values after loading.
			DBusBasicValue * values = nullptr;

		};
 	}
 
 }

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
  * @brief Declare dbus-.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <udjat/action.h>
 #include <udjat/tools/abstract/object.h>
 #include <udjat/tools/string.h>
 #include <dbus/dbus.h>
 #include <memory>
 #include <vector>

 namespace Udjat {

	namespace DBus {

		class UDJAT_API Action : public Udjat::Action {
		public:

			class UDJAT_API Argument {
			private:
				const char *name;				///< @brief Argument name.
				const char *tmplt = nullptr;	///< @brief Template for argument value.
				String value;					///< @brief The argument value.
				int type;						///< @brief The dbus type for this argument.

			public:
				Argument(const XML::Node &node);

				const char * c_str() const noexcept {
					return name;
				}

				const DBusBasicValue * get(DBusBasicValue &dbval) const;

				/// @brief Get argument from Udjat::Value.
				/// @param value The Udjat value to get data from.
				/// @param dbval The dbus value to set.
				const DBusBasicValue * set(const Udjat::Value &value, DBusBasicValue &dbval);

				/// @brief Get argument from abstract object.
				/// @param object The object to get properties from.
				/// @param dbval The dbus value to set.
				const DBusBasicValue * set(const Udjat::Abstract::Object &object, DBusBasicValue &dbval);

				static int TypeFactory(const XML::Node &node);

				inline int dbus_type() const noexcept {
					return type;
				}

			};

			class Factory : public Udjat::Action::Factory {
			public:
				Factory(const char *name = "dbus") : Udjat::Action::Factory{name} {
				}

				std::shared_ptr<Udjat::Action> ActionFactory(const XML::Node &node) const override;

			};

			Action(const XML::Node &node);

			/// @brief Set properties from request, call action and set it as active.
			/// @param request The request to get properties from.
			/// @param response The response received from d-bus service.
			/// @param except If true, launch exceptions on errors.
			/// @return 0 if ok, error code otherwise.
			int call(Udjat::Request &request, Udjat::Response &response, bool except) override;

			/// @brief Call action using already set parameters.
			/// @param except If true, launch exceptions on errors.
			/// @return 0 if ok, error code otherwise.
			int call(bool except) override;

			void introspect(std::stringstream &xmldata) const;

		protected:

			/// @brief The d-bus message type.
			int message_type;

			/// @brief Get message type from XML node.
			/// @param node XML node.
			/// @param def The default message type.
			/// @param attrname The xml attribute name.
			/// @return 
			static int MessageTypeFactory(const XML::Node &node, const char *def = "signal", const char *attrname = "dbus-message-type");

			/// @brief The bus type.
			DBusBusType bustype = DBUS_BUS_STARTER;

			/// @brief The d-bus service name.
			const char *service;

			/// @brief The path to the object.
			const char *path;

			/// @brief The d-bus interface name.
			const char *interface;

			/// @brief The d-bus member name.
			const char *member;

			std::vector<Argument> arguments;

			std::shared_ptr<DBusMessage> MessageFactory() const;
				
			void get_arguments(std::shared_ptr<DBusMessage> message, const Udjat::Request &request);
			void get_arguments(std::shared_ptr<DBusMessage> message, const Udjat::Abstract::Object &object);

			/// @brief Set arguments from Udjat::Abstract::Object, change state to active.
			/// @param object Object to get properties from.
			/// @return true
			bool activate(const Udjat::Abstract::Object &object) noexcept override;

		};

	}

 }
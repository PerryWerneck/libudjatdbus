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
  * @brief Declare d-bus emitter.
  */

 #pragma once

 #include <udjat/defs.h>
 #include <udjat/tools/abstract/object.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/string.h>
 #include <vector>
 #include <dbus/dbus.h>

 namespace Udjat {

	namespace DBus {

		/// @brief Emits a d-bus signal or call a method.
		class UDJAT_API Emitter {
		private:

			/// @brief The d-bus message type.
			int message_type;

			/// @brief The bus type for alert.
			DBusBusType bustype = DBUS_BUS_STARTER;

			/// @brief The path to the object emitting the alert.
			const char *path = nullptr;

			/// @brief The interface the alert is emitted from.
			const char *iface = nullptr;

			/// @brief The alert member.
			const char *member = nullptr;

			/// @brief D-Bus alert inputs.
			struct Input {
				const char *name;		///< @brief Argument name.
				int type;				///< @brief D-Bus data type.
				DBusBasicValue dbval;	///< @brief Default value.

				Input(const XML::Node &node);

			};
			std::vector<Input> inputs;

			/// @brief D-Bus message arguments.
			struct Argument {
				int type;				///< @brief D-Bus data type.
				String value;			///< @brief Argument string.
				DBusBasicValue dbval;	///< @brief Argument value.

				Argument(int t, const char *s) : type{t}, value{s} {
				}

				Argument(int t, const DBusBasicValue &v) : type{t}, dbval{v} {
				}

			};
			std::vector<Argument> arguments;

			struct {
				String path;
				String iface;
				String member;
			} activation;

		protected:

			void prepare();
			void prepare(const Abstract::Object &object);

			/// @brief Send message with arguments.
			void send();

			/// @brief Clear arguments.
			void clear() noexcept;

		public:
			Emitter(const XML::Node &node);

		};

	}

 }

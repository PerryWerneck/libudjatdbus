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
  * @brief Declare an interface inside the d-bus connection.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <string>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/dbus/member.h>
 #include <list>
 #include <functional>

 namespace Udjat {

	namespace Abstract {

		namespace DBus {

			class UDJAT_API Interface : public std::string {
			protected:
				const char *type;

			public:

				/// @brief Get interface name from XML node.
				/// @param node Node with interface definition.
				/// @return The interface name.
				static Udjat::String NameFactory(const XML::Node &node);

				virtual DBusHandlerResult filter(DBusMessage *message) const = 0;

				Interface(const char *name, const char *type = "signal");
				Interface(const XML::Node &node, const char *type = "signal");
				virtual ~Interface();

				bool operator==(const char *intf) const noexcept;

				/// @brief Get textual form of match rule for this interface.
				const std::string rule() const;

			};

		}

	}

	namespace DBus {

		class UDJAT_API Interface : public Abstract::DBus::Interface {
		private:
			std::list<Udjat::DBus::Member> members;

		public:

			virtual DBusHandlerResult filter(DBusMessage *message) const override;

			Interface(const char *name, const char *type = "signal") : Abstract::DBus::Interface{name,type} {
			}

			Interface(const XML::Node &node, const char *type = "signal"): Abstract::DBus::Interface{node,type} {
			}

			virtual ~Interface();

			inline bool empty() const noexcept {
				return members.empty();
			}

			Udjat::DBus::Member & push_back(const XML::Node &node,const std::function<bool(Message & message)> &callback);
			Udjat::DBus::Member & emplace_back(const char *member, const std::function<bool(Message & message)> &callback);

			void remove(const Udjat::DBus::Member &member);

			inline auto begin() const noexcept {
				return members.begin();
			}

			inline auto end() const noexcept {
				return members.end();
			}

		};

	}

 }

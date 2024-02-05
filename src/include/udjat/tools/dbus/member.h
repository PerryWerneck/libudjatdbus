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
  * @brief Declare dbus-member.
  */

 #pragma once

 #pragma once
 #include <udjat/defs.h>
 #include <udjat/tools/dbus/message.h>
 #include <string>
 #include <functional>
 #include <udjat/tools/xml.h>

 namespace Udjat {

	namespace DBus {

		class UDJAT_API Member : public std::string {
		private:
			const std::function<void(Message & message)> &callback;

		public:
			Member(const char *name,const std::function<void(Message & message)> &callback);
			Member(const XML::Node &node,const std::function<void(Message & message)> &callback);
			~Member();

			bool operator==(const char *name) const noexcept;

			inline void call(Message &message) const {
				callback(message);
			}

		};

	}

 }

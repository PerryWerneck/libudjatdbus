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
  * @brief Declare D-Bus Exception.
  */


 #pragma once
 #include <udjat/defs.h>
 #include <dbus/dbus.h>
 #include <stdexcept>

 namespace Udjat {

	namespace DBus {

		class UDJAT_API Exception : public std::runtime_error {
		private:
			DBusMessage *error_message;

		public:
			Exception(DBusMessage *message, const char *error_name, const char *error_text = nullptr);
			~Exception();

			void send(DBusConnection *connct) const noexcept;

		};

		class UDJAT_API Error {
		private:
			DBusError err;

		public:
			Error() {
				dbus_error_init(&err);
			}

			~Error() {
				dbus_error_free(&err);
			}

			inline operator DBusError *() noexcept {
				return &err;
			}

			/// @brief Throw exception if error is set.
			void verify();

		};

	}
 }



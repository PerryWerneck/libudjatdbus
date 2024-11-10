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
  * @brief Declares DBus::Signal.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <string>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/dbus/connection.h>
 #include <dbus/dbus.h>

 namespace Udjat {

	namespace DBus {

		class UDJAT_API Signal {
		private:

			/// @brief The D-Bus message.
			DBusMessage *message;

			/// @brief The message iter.
			DBusMessageIter iter;

			inline Signal & add() noexcept {
				return *this;
			}

			template<typename T, typename... Targs>
			Signal & add(T &value, Targs... Fargs) {
				push_back(value);
				return add(Fargs...);
			}

		public:
			Signal(const char *iface, const char *member, const char *path);

			template<typename T, typename... Targs>
			Signal(const char *iface, const char *member, const char *path, const T &value, Targs... Fargs)
				: Signal(iface,member,path) {
				push_back(value);
				add(Fargs...);
			}

			~Signal();

			inline DBusMessage * dbus_message() const noexcept {
				return message;
			}

			/// @brief Emit signal to the system bus.
			void system();

			/// @brief Emit signal to the session bus.
			void session();

			/// @brief Emit signal to the starter bus.
			void starter();

			/// @brief Emit signal to the connection.
			void emit(Abstract::DBus::Connection &connection);

			/// @brief Emit signal directly to selected user bus.
			void user(uid_t uid, const char *sid = "");

			/// @brief Add values to signal.
			Signal & push_back(const char *value);

			inline Signal & push_back(const std::string &value) {
				return push_back(value.c_str());
			}

			Signal & push_back(const bool value);

			Signal & push_back(const int16_t value);
			Signal & push_back(const uint16_t value);

			Signal & push_back(const int32_t value);
			Signal & push_back(const uint32_t value);

			Signal & push_back(const int64_t value);
			Signal & push_back(const uint64_t value);

		};

	}

 }

 template <typename T>
 inline Udjat::DBus::Signal & operator<<(Udjat::DBus::Signal &signal, const T value) {
	return signal.push_back(value);
 }

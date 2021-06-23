/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2021 Perry Werneck <perry.werneck@gmail.com>
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

 #include <udjat/defs.h>
 #include <udjat/tools/value.h>
 #include <dbus/dbus.h>
 #include <stdexcept>
 #include <string>
 #include <list>

 namespace Udjat {

	namespace DBus {

		class Connection;

		class UDJAT_API Error : public DBusError {
		public:
			Error() {
				dbus_error_init(this);
			}

			~Error() {
				dbus_error_free(this);
			}

			void clear() {
				dbus_error_free(this);
			}

			operator bool() {
				return dbus_error_is_set(this);
			}

			void test() {
				if(dbus_error_is_set(this)) {
					throw std::runtime_error(this->message);
				}
			}
		};

		/// @brief D-Bus worker.
		class UDJAT_API Worker {
		protected:

			/// @brief Message type (from dbus_message_get_type)
			int type = -1;

			/// @brief Member (from dbus_message_get_member)
			const char *member = nullptr;

			/// @brief Interface (from dbus_message_get_interface);
			const char *interface = nullptr;

		public:
			Worker();
			virtual ~Worker();

			/// @brief Check if the worker is assigned to the message.
			bool equal(const DBusMessage *message);

			/// @brief Process message
			void work(Connection *controller, DBusMessage *message);

		};

		/// @brief D-Bus Connection.
		class UDJAT_API Connection {
		private:
			DBusConnection	* connct;
			std::string		  name;

			static DBusHandlerResult filter(DBusConnection *connection, DBusMessage *message, Connection *connct) noexcept;

			std::list<Worker *> workers;

		public:
			Connection(DBusBusType type);
			~Connection();

			/// @brief Asks the bus to assign the given name to this connection by invoking the RequestName method on the bus.
			Connection & request(const char *name, unsigned int flags = DBUS_NAME_FLAG_REPLACE_EXISTING);

			void insert(Worker *worker);
			void remove(Worker *worker);
		};

		/// @brief D-Bus value.
		class UDJAT_API Value : public Udjat::Value {
		private:
			int type;
			DBusBasicValue value;

		public:
			Value();
			~Value();

			Udjat::Value & reset(const Type type) override;

			Udjat::Value & set(const char *value, const Type type) override;
			Udjat::Value & set(const short value) override;
			Udjat::Value & set(const unsigned short value) override;
			Udjat::Value & set(const int value) override;
			Udjat::Value & set(const unsigned int value) override;
			Udjat::Value & set(const long value) override;
			Udjat::Value & set(const unsigned long value) override;
			Udjat::Value & set(const TimeStamp value) override;
			Udjat::Value & set(const bool value) override;
			Udjat::Value & set(const float value) override;
			Udjat::Value & set(const double value) override;

		};

	}

 }



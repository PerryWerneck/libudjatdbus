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
 #include <udjat/request.h>
 #include <dbus/dbus.h>
 #include <stdexcept>
 #include <string>
 #include <list>
 #include <map>
 #include <mutex>
 #include <thread>

 namespace Udjat {

	namespace DBus {

		class Connection;
		class Response;

		/// @brief D-Bus value.
		class UDJAT_API Value : public Udjat::Value {
		private:
			friend class Response;

			int type;
			DBusBasicValue value;
			std::map<std::string,Value *> children;

		public:
			Value();
			virtual ~Value();

			/// @brief Add value on the message.
			void get(DBusMessageIter *iter);

			/// @brief The value has children?
			inline bool empty() const noexcept {
				return children.empty();
			}

			Udjat::Value & reset(const Udjat::Value::Type type = Udjat::Value::Undefined) override;

			bool isNull() const override;

			Udjat::Value & operator[](const char *name) override;

			Udjat::Value & append(const Type type) override;
			Udjat::Value & set(const Udjat::Value &value) override;

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

		/// @brief D-Bus request.
		class Request : public Udjat::Request {
		private:
			DBusMessage *message;
			DBusMessageIter iter;

			int pop(DBusBasicValue &value);

		public:
			Request(DBusMessage *message);
			virtual ~Request();

			std::string pop() override;
			Udjat::Request & pop(int &value) override;
			Udjat::Request & pop(unsigned int &value) override;

		};

		/// @brief D-Bus response.
		class Response : public Udjat::Response {
		private:
			DBus::Value value;
			Connection *connct;

		public:
			Response(Connection *connct);
			virtual ~Response();

			/// @brief Load message reply.
			/// @param message Message reply (from dbus_message_new_method_return)
			void get(DBusMessage *message);

			bool isNull() const override;
			Udjat::Value & operator[](const char *name) override;
			Udjat::Value & append(const Type type = Object) override;
			Udjat::Value & reset(const Type type = Undefined) override;
			Udjat::Value & set(const Value &value) override;
			Udjat::Value & set(const char *value, const Type type = String) override;

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
			bool equal(DBusMessage *message);

			/// @brief Execute request.
			virtual void work(DBus::Request &request, DBus::Response &response);

			/// @brief Process message
			void work(Connection *controller, DBusMessage *message);

		};

		/// @brief D-Bus Connection.
		class UDJAT_API Connection {
		private:

			/// @brief Mutex for serialization.
			std::recursive_mutex guard;

			/// @brief D-Bus connection mainloop.
			std::thread		* mainloop = nullptr;

			/// @brief Dispatcher
			// static dispatcher(Connection *connct);

			/// @brief D-Bus connection handle.
			DBusConnection	* connct = NULL;

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

	}

 }



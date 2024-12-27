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
  * @brief Declare D-Bus connection.
  */

 #pragma once

 #include <dbus/dbus.h>
 #include <udjat/defs.h>
 #include <udjat/tools/dbus/defs.h>
 #include <string>
 #include <mutex>
 #include <thread>
 #include <list>
 #include <udjat/tools/xml.h>

 namespace Udjat {

 	namespace DBus {

		/// @brief Initialize D-Bus system.
		bool UDJAT_API initialize();

		DBusBusType UDJAT_API BusTypeFactory(const XML::Node &node);

		/// @brief Connection to D-Bus service.
		class UDJAT_API Connection {
		private:

			/// @brief The connection name.
			std::string object_name;

			/// @brief Service thread.
			std::thread * thread = nullptr;

			/// @brief Message filter method.
			static DBusHandlerResult on_message(DBusConnection *, DBusMessage *, Connection *) noexcept;

			/// @brief Interfaces in this connection.
			std::list<Interface> interfaces;

			void insert(const Interface &interface);
			void remove(const Interface &interface);

		protected:

			/// @brief Connection to D-Bus.
			DBusConnection * conn = nullptr;

			/// @brief Mutex for serialization.
			static std::mutex guard;

			Connection(const char *name, DBusConnection * conn);

			/// @brief Filter message
			virtual DBusHandlerResult filter(DBusMessage *message);

			/// @brief Registers a connection with the bus.
			void bus_register();

			/// @brief Asks the bus to assign the given name to this connection by invoking the RequestName method on the bus.
			int request_name(const char *name);

			void clear();

		public:

			/// @brief Get connection from XML node
			/// @param node The XML node describing the connection.
			/// @return The d-bus connection.
			static Connection & getInstance(const XML::Node &node);

			/// @brief Get connection from bus type.
			/// @param bustype The type for the connection.
			/// @return The d-bus connection.
			static Connection & getInstance(const DBusBusType bustype = DBUS_BUS_STARTER);

			inline operator bool() const noexcept {
				return (bool) conn;
			}

			inline const char * name() const noexcept {
				return object_name.c_str();
			}

			inline DBusConnection * connection() const noexcept {
				return conn;
			}

			inline operator DBusConnection *() const noexcept {
				return conn;
			}

			virtual ~Connection();

			void flush() noexcept;

			void push_back(Interface &interface);
			void remove(Interface &interface);
			
			void push_back(const XML::Node &node);

			Interface & emplace_back(const char *interface);

			inline auto begin() const {
				return interfaces.begin();
			}

			inline auto end() const {
				return interfaces.end();
			}

			/// @brief Emit signal.
			void signal(const Signal &sig);

			/// @brief Subscribe to d-bus signal.
			/// @return Member handling the signal.
			Member & subscribe(const char *interface, const char *member, const std::function<bool(Message &message)> &callback);

			/// @brief Watch d-bus method calls.
			/// @return Member handling the signal.
			Member & watch(const char *interface, const char *member, const std::function<bool(Message &message)> &callback);

			/// @brief Unsubscribe from d-bus signal.
			void remove(const Member &member);

			/// @brief Call method
			void call(DBusMessage * message, const std::function<void(Message & message)> &call);

			/// @brief Call method (syncronous);
			void call(DBusMessage * message);

			/// @brief Call method (syncronous);
			void call_and_wait(DBusMessage * message, const std::function<void(Message & message)> &call);

			/// @brief Call method (async)
			void call(	const char *destination,
						const char *path,
						const char *interface,
						const char *member,
						const std::function<void(Message & message)> &call
					);

			/// @brief Call method (async)
			void call(	const Message & request,
						const std::function<void(Message & response)> &call
					);

			/// @brief Call method (sync)
			void call_and_wait(	const char *destination,
								const char *path,
								const char *interface,
								const char *member,
								const std::function<void(Message & message)> &call
					);

			void call_and_wait(	const Message & request,
								const std::function<void(Message & response)> &call
					);

			/// @brief Get property.
			/// @param name	Property name.
			void get(	const char *destination,
						const char *path,
						const char *interface,
						const char *property_name,
						const std::function<void(Message & message)> &call
					);

		};

		/// @brief System bus connection.
		class UDJAT_API SystemBus : public Connection {
		public:
			static DBusConnection * ConnectionFactory();

			SystemBus();
			virtual ~SystemBus();

			static Connection & getInstance();

		};

		/// @brief D-Bus shared connection to session bus.
		class UDJAT_API SessionBus : public Connection {
		public:
			static DBusConnection * ConnectionFactory();

			SessionBus();
			virtual ~SessionBus();

			static Connection & getInstance();

		};

		/// @brief D-Bus shared connection to starter bus.
		class UDJAT_API StarterBus : public Connection {
		public:
			static DBusConnection * ConnectionFactory();

			StarterBus();
			virtual ~StarterBus();

			static Connection & getInstance();

		};

		/// @brief Private connection to a named bus.
		class UDJAT_API NamedBus : public Connection {
		protected:
			NamedBus(const char *name, DBusConnection * conn);

		public:
			/// @param connection_name The object name (for logging).
			/// @param address The D-Bus Address for this connection.
			NamedBus(const char *connection_name, const char *address);
			virtual ~NamedBus();

		};

		/// @brief Private connection to an user's bus.
		class UDJAT_API UserBus : public NamedBus {
		protected:
			uid_t userid;

		public:
			UserBus(uid_t uid, const char *sid = "");

			/// @brief Execute function as user's effective id, serialize to avoid conflicts.
			static void exec(uid_t uid, const std::function<void()> &func);

			/// @brief Execute function as user's effective id.
			inline void exec(const std::function<void()> &func) const {
				exec(userid,func);
			};

		};

 	}

 }

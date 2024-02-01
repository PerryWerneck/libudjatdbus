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

 /*
 #include <udjat/defs.h>
 #include <udjat/tools/value.h>
 #include <ostream>
 #include <dbus/dbus.h>
 #include <mutex>
 #include <functional>
 #include <list>
 #include <map>
 #include <thread>
 #include <vector>
 #include <udjat/tools/dbus/message.h>
 #include <udjat/tools/dbus/value.h>

 namespace Udjat {

	namespace DBus {

		/// @brief D-Bus connection.
		class UDJAT_API Connection {
		private:
			friend class System;
			friend class Session;
			friend class Starter;

			static DBusConnection * Factory(DBusBusType type);
			static DBusConnection * Factory(uid_t uid, const char *sid);

			/// @brief Connection name.
			std::string name;

			/// @brief Mutex for serialization.
			static std::recursive_mutex guard;

			/// @brief Conexão ao barramento D-Bus.
			DBusConnection * connection = nullptr;

			/// @brief Using threads?
			bool use_thread = false;

			/// @brief Service thread.
			std::thread * thread = nullptr;

			/// @brief Handle signal
			DBusHandlerResult on_signal(DBusMessage *message);

			/// @brief D-Bus signal listener
			struct Listener {
				void *id;
				std::string name;
				std::function<void(Message & message)> call;

				Listener(void *i, const std::string &n, std::function<void(Message & message)> c)
					: id(i), name(n), call(c) { }
			};

			/// @brief Interface no barramento D-Bus.
			struct Interface {
				std::string name;
				std::list<Listener> members;
				Interface(const char *n) : name(n) { }

				inline bool empty() const noexcept {
					return members.empty();
				}

				/// @brief Obtém string de "match" para o nome informado.
				static std::string getMatch(const char *name);

				/// @brief Obtém string de "match" para a interface.
				inline std::string getMatch() const {
					return getMatch(name.c_str());
				}

				/// @brief Unsubscribe by id.
				bool unsubscribe(const DBus::Connection *connection, void *id);

				/// @brief Unsubscribe by memberName.
				bool unsubscribe(const DBus::Connection *connection, void *id, const char *memberName);

				/// @brief Remove interface from connection.
				void remove_from(const DBus::Connection * connection) noexcept;
			};

			/// @brief Subscribed interfaces.
			std::list<Interface> interfaces;

			/// @brief Obtém interface pelo nome, inclui se for preciso.
			Interface & getInterface(const char *name);

			/// @brief Message filter method.
			static DBusHandlerResult filter(DBusConnection *, DBusMessage *, DBus::Connection *);

			Connection(DBusConnection * connection, const char *name = "d-bus", bool reg = true);

		public:

			/// @brief Get singleton connection to system bus for root user, user bus for others.
			static Connection & getInstance();

			/// @brief Get singleton connection to the system bus.
			static System & getSystemInstance();

			/// @brief Get singleton connection to the session bus.
			static Session & getSessionInstance();

			/// @brief Get singleton connection to the starter bus.
			static Starter & getStarterInstance();

			inline operator bool() const {
				return this->connection != nullptr;
			}

			/// @brief Get connection to a named bus.
			Connection(const char *busname, const char *connection_name = "d-bus");

			/// @brief Get connection to user bus.
			/// @param uid User ID.
			/// @param sid Session ID.
			Connection(uid_t uid, const char *sid = nullptr);

			~Connection();

			Connection(const Connection &) = delete;
			Connection(const Connection *) = delete;

			inline DBusConnection * getConnection() const {
				return connection;
			}

			void flush() noexcept;

			/// @brief Adds a message to the outgoing message queue.
			/// @param message The message to add.
			inline dbus_bool_t send(DBusMessage *message) const {
				return dbus_connection_send(connection,message,NULL);
			}

			/// @brief Blocks until the outgoing message queue is empty.
			inline void flush() const {
				return dbus_connection_flush(connection);
			}

			/// @brief Get connection name.
			inline const char * c_str() const noexcept {
				return name.c_str();
			}

			/// @brief Dispatcher
			static void dispatch(DBusConnection * connection) noexcept;

			/// @brief Subscribe to D-Bus signal.
			void subscribe(void *id, const char *interface, const char *member, std::function<void(DBus::Message &message)> call);

			/// @brief Unsubscribe from D-Bus signal.
			void unsubscribe(void *id, const char *interfaceName, const char *memberName);

			/// @brief Unsubscribe all signals created by 'id'.
			void unsubscribe(void *id);

			/// @brief Call method
			void call(DBusMessage * message, const std::function<void(Message & message)> &call);

			/// @brief Call method (syncronous);
			void call(DBusMessage * message);

			/// @brief Call method (syncronous);
			void call_and_wait(DBusMessage * message, const std::function<void(Message & message)> &call);

			/// @brief Call method
			void call(	const char *destination,
						const char *path,
						const char *interface,
						const char *member,
						std::function<void(Message & message)> call
					);

			/// @brief Call method
			void call(	const Message &message,
						std::function<void(Message & message)> call
					);

			std::ostream & info() const;
			std::ostream & warning() const;
			std::ostream & error() const;
			std::ostream & trace() const;

		};

		/// @brief System Bus.
		class UDJAT_API System : public Connection {
		public:
			System();
		};


		/// @brief Session Bus.
		class UDJAT_API Session : public Connection {
		public:
			Session();
		};

		/// @brief Starter Bus.
		class UDJAT_API Starter : public Connection {
		public:
			Starter();
		};

		/// @brief D-Bus signal
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

			/// @brief Emit signal to the system bus.
			void system();

			/// @brief Emit signal to the session bus.
			void session();

			/// @brief Emit signal to the starter bus.
			void starter();

			void send();

			/// @brief Emit the signal to the connection.
			void send(Connection &connection);

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

 template <typename T>
 inline Udjat::DBus::Message & operator<<(Udjat::DBus::Message &message, const T value) {
	return message.push_back(value);
 }
 */

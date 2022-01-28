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
 #include <mutex>
 #include <functional>
 #include <list>
 #include <map>
 #include <thread>

 namespace Udjat {

	namespace DBus {

		class Connection;
		class Message;

		/// @brief D-Bus Value
		class UDJAT_API Value : public Udjat::Value {
		private:

			/// @brief D-Bus data type.
			int type;

			/// @brief D-Bus value.
			DBusBasicValue value;

			/// @brief Value children.
			std::map<std::string,Value *> children;

			/// @brief Check if the value dont have a signagture.
			/// @return true if the value can be added on signatured.
			inline bool noSignature() const noexcept {
				return (type == DBUS_TYPE_INVALID || type == DBUS_TYPE_ARRAY || type == DBUS_TYPE_DICT_ENTRY);
			}

			/// @brief Get signature for array export.
			std::string getArraySignature() const noexcept;

		public:

			// String values have an strdup; the copy can invalidate the pointer.
			Value(const Value *src);
			Value(const Value &src);
			Value(Message &message);

			Value();
			Value(int type, const char *value);
			virtual ~Value();

			/// @brief Add value on iter.
			void get(DBusMessageIter *iter) const;

			/// @brief Set value from iter.
			/// @return true if the value is valid.
			bool set(DBusMessageIter *iter);

			/// @brief The value has children?
			inline bool empty() const noexcept {
				return children.empty();
			}

			inline bool operator==(int type) const noexcept {
				return this->type == type;
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

			const Udjat::Value & get(std::string &value) const override;
			// const Udjat::Value & get(short &value) const override;
			// const Udjat::Value & get(unsigned short &value) const override;
			// const Udjat::Value & get(int &value) const override;
			const Udjat::Value & get(unsigned int &value) const override;
			// const Udjat::Value & get(long &value) const override;
			// const Udjat::Value & get(unsigned long &value) const override;
			// const Udjat::Value & get(TimeStamp &value) const override;
			const Udjat::Value & get(bool &value) const override;
			// const Udjat::Value & get(float &value) const override;
			// const Udjat::Value & get(double &value) const override;

		};

		/// @brief D-Bus message
		class UDJAT_API Message {
		private:

			struct {
				DBusMessage *value = nullptr;
				DBusMessageIter iter;
			} message;

			struct {
				bool valid = false;		/// @brief True if this is an error message.
				std::string name;		/// @brief Error name.
				std::string message;	/// @brief Error Message.
			} error;

		public:
			Message(const Message &message) = delete;
			Message(const Message *message) = delete;

			Message(const DBusError &error);
			Message(DBusMessage *m);

			~Message();

			inline operator bool() const {
				return !error.valid;
			}

			Message & pop(Value &value);

			DBusMessageIter * getIter();

			inline bool failed() const {
				return error.valid;
			}

			bool next();

			template <typename T>
			Message & pop(T &value) {
				Value v;
				pop(v);
				v.get(value);
				return *this;
			}

			inline const char * error_name() const {
				return this->error.name.c_str();
			}

			inline const char * error_message() const {
				return error.message.c_str();
			}

		};

		/// @brief D-Bus connection.
		class UDJAT_API Connection {
		private:

			/// @brief Connection name.
			std::string name;

			/// @brief Semaforo para serializar acessos.
			static std::recursive_mutex guard;

			/// @brief Conexão ao barramento D-Bus.
			DBusConnection * connection = nullptr;

			/// @brief Thread de serviço D-Bus.
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

			// void removeMatch(DBus::Connection::Interface &interface);

			/// @brief Obtém interface pelo nome, inclui se for preciso.
			Interface & getInterface(const char *name);

			/// @brief Message filter method.
			static DBusHandlerResult filter(DBusConnection *, DBusMessage *, DBus::Connection *);

			Connection(DBusConnection * connection, const char *name = "d-bus", bool reg = true);

		public:

			/// @brief Get singleton connection to system bus for root user, user bus for others.
			static Connection & getInstance();

			/// @brief Get singleton connection to the system bus.
			static Connection & getSystemInstance();

			/// @brief Get singleton connection to the session bus.
			static Connection & getSessionInstance();

			/// @brief Get connection to a named bus.
			Connection(const char *busname, const char *connection_name = "d-bus");
			~Connection();

			Connection(const Connection &) = delete;
			Connection(const Connection *) = delete;

			inline DBusConnection * getConnection() const {
				return connection;
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

			/// @brief call method
			void call(	const char *destination,
						const char *path,
						const char *interface,
						const char *member,
						std::function<void(Message & message)> call
					);

		};

	}

 }

 /*
 template <typename T>
 inline Udjat::Value & operator<<(Udjat::DBus::Message &out, T value) {
	return out.set(value);
 }

 template <typename T>
 inline Udjat::Value & operator>> (const Udjat::DBus::Message &in, T &value ) {
	in.get(value);
	return in;
 }
 */


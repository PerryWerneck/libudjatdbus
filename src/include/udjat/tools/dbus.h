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
 #include <dbus/dbus.h>
 #include <mutex>
 #include <functional>
 #include <list>
 #include <thread>

 namespace Udjat {

	namespace DBus {

		/// @brief D-Bus interface
		class Interface;

		/// @brief Conexão com o barramento D-Bus.
		class Connection {
		private:

			/// @brief Semaforo para serializar acessos.
			static std::mutex guard;

			/// @brief Conexão ao barramento D-Bus.
			DBusConnection * connection = nullptr;

			/// @brief Thread de serviço D-Bus.
			std::thread * thread = nullptr;

			void start();

			/// @brief True se a conexão está ativa.
			bool active = false;

			void set(DBusConnection * connection);

			/// @brief Handle signal
			DBusHandlerResult on_signal(DBusMessage *message);

			/// @brief Método para responder a um sinal D-Bus.
			struct Listener {
				std::string name;
				std::function<void(DBusMessage * message)> call;

				Listener(const std::string &n, std::function<void(DBusMessage * message)> c)
					: name(n), call(c) { }
			};

			/// @brief Interface no barramento D-Bus.
			struct Interface {
				std::string name;
				std::list<Listener> members;
				Interface(const char *n) : name(n) { }

				/// @brief Obtém string de "match" para o nome informado.
				static std::string getMatch(const char *name);

				/// @brief Obtém string de "match" para a interface.
				inline std::string getMatch() const {
					return getMatch(name.c_str());
				}

			};

			/// @brief Subscribed interfaces.
			std::list<Interface> interfaces;

			/// @brief Obtém interface pelo nome, inclui se for preciso.
			Interface & getInterface(const char *name);

			/// @brief Message filter method.
			static DBusHandlerResult filter(DBusConnection *, DBusMessage *, Connection *);

		public:

			static Connection & getSystemInstance();
			static Connection & getSessionInstance();

			Connection();
			Connection(const char *busname);
			Connection(DBusConnection * connection);
			Connection(DBusBusType type);
			~Connection();

			DBusConnection * getConnection() const {
				return connection;
			}

			/// @brief Subscribe to D-Bus signal.
			void subscribe(void *id, const char *interface, const char *member, std::function<void(DBusMessage * message)> call);

			/// @brief Unsubscribe from D-Bus signal.
			void unsubscribe(void *id, const char *interfaceName, const char *memberName);

			/// @brief Unsubscribe all signals created by 'id'.
			void unsubscribe(void *id);

			/*
			/// @brief call method
			void call(	const char *destination,
						const char *path,
						const char *interface,
						const char *member,
						std::function<void(DBusMessage * message, DBusError *error)> call
					);
			*/

		};

	}

 }



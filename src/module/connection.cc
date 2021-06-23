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

 #include "private.h"

 namespace Udjat {

	namespace DBus {

		Connection::Connection(DBusBusType type) {

			Error error;

			connct = dbus_bus_get(type,&error);
			error.test();

			dbus_connection_set_exit_on_disconnect(connct, false);

			if (dbus_connection_add_filter(connct, (DBusHandleMessageFunction) filter, this, NULL) == FALSE) {
				dbus_connection_unref(connct);
				throw runtime_error("Unable to add signal filter");
			}

		}

		Connection::~Connection() {

			dbus_connection_remove_filter(connct,(DBusHandleMessageFunction) filter, this);

			if(!name.empty()) {

				Error error;
				dbus_bus_release_name(connct,name.c_str(),&error);

				if(error) {
					cerr << error.message << endl;
				}

			}

			dbus_connection_unref(connct);

		}

		Connection & Connection::request(const char *name, unsigned int flags) {

			if(!this->name.empty()) {
				throw std::system_error(EBUSY, std::system_category(), "This connection already has a name");
			}

			Error error;
			dbus_bus_request_name(connct,name,flags,&error);
			error.test();

			return *this;
		}


		DBusHandlerResult Connection::filter(DBusConnection *connection, DBusMessage *message, Connection *controller) noexcept {

			/*
			if(dbus_message_get_type(message) != DBUS_MESSAGE_TYPE_SIGNAL) {
				return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
			}

			std::lock_guard<std::recursive_mutex> lock(mtx);

			const char *member		= dbus_message_get_member(message);
			const char *interface	= dbus_message_get_interface(message);
			for(auto intf : obj->interfaces) {

				if(strcmp(intf->c_str(),interface)) {
					continue;
				}

				for(auto memb : intf->members) {

					if(strcmp(memb.c_str(),member)) {
						debug("Ignorando membro %s",memb.c_str());
						continue;
					}

					// Achei o membro, executa!
					try {

						_msg req(message);
						debug("Executando sinal \"%s.%s\"",interface,member);
						memb.call(req);
						return DBUS_HANDLER_RESULT_HANDLED;

					} catch( const std::exception &e ) {

						error("%s.%s: %s",interface,member,e.what());
						return DBUS_HANDLER_RESULT_HANDLED;

					} catch( ... ) {

						error("%s.%s: %s",interface,member,"Erro inesperado");
						return DBUS_HANDLER_RESULT_HANDLED;

					}
				}

			}

			*/

			return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

		}

	}

 }


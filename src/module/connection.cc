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

		Connection & Connection::getInstance() {
#ifdef DEBUG
			static Connection instance(DBUS_BUS_SESSION);
#else
			static Connection instance(DBUS_BUS_SYSTEM);
#endif // DEBUG

			return instance;

		}

		Connection::Connection(DBusBusType type) {

			Error error;

			lock_guard<recursive_mutex> lock(guard);

			connct = dbus_bus_get(type,&error);
			error.test();

			dbus_connection_set_exit_on_disconnect(connct, false);

			if (dbus_connection_add_filter(connct, (DBusHandleMessageFunction) filter, this, NULL) == FALSE) {
				dbus_connection_unref(connct);
				throw runtime_error("Unable to add signal filter");
			}

			mainloop = new thread([this]() {

				DBusConnection * c = this->connct;

				{
					lock_guard<recursive_mutex> lock(guard);
					dbus_connection_ref(c);
				}

				while(this->connct && dbus_connection_read_write(c,500)) {

					lock_guard<recursive_mutex> lock(guard);
					while(dbus_connection_get_dispatch_status(c) == DBUS_DISPATCH_DATA_REMAINS) {
						dbus_connection_dispatch(c);
					}

				}

				{
					lock_guard<recursive_mutex> lock(guard);
					dbus_connection_unref(c);
				}

			});

		}

		Connection::~Connection() {

			{
				lock_guard<recursive_mutex> lock(guard);
				dbus_connection_remove_filter(connct,(DBusHandleMessageFunction) filter, this);

				if(!name.empty()) {

					Error error;
					dbus_bus_release_name(connct,name.c_str(),&error);

					if(error) {
						cerr << error.message << endl;
					}

				}

				dbus_connection_unref(connct);
				connct = nullptr;
			}

			// Wait for mainloop to end.
			cout << "d-bus\tWaiting for connection to finish" << endl;
			mainloop->join();
			delete mainloop;

		}

		Connection & Connection::request(const char *name, unsigned int flags) {

			lock_guard<recursive_mutex> lock(guard);

			if(!this->name.empty()) {
				throw std::system_error(EBUSY, std::system_category(), "This connection already has a name");
			}

			Error error;
			dbus_bus_request_name(connct,name,flags,&error);
			error.test();

			return *this;
		}


	}

 }


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
 #include <udjat/tools/mainloop.h>

 namespace Udjat {

	namespace DBus {

		static dbus_bool_t add_watch(DBusWatch *watch, DBusConnection *conn) {

			unsigned int flags = dbus_watch_get_flags(watch);

			int event = 0;

			if (flags & DBUS_WATCH_READABLE)
				event |= MainLoop::oninput;

			if (flags & DBUS_WATCH_WRITABLE)
				event |= MainLoop::onoutput;

			MainLoop::getInstance().insert(
				watch,
				dbus_watch_get_unix_fd(watch),
				(MainLoop::Event) event,
				[watch, conn](const MainLoop::Event events) {

					unsigned int flags = 0;

					if (events & MainLoop::oninput)
						flags |= DBUS_WATCH_READABLE;

					if (events & MainLoop::onoutput)
						flags |= DBUS_WATCH_WRITABLE;

					if (events & MainLoop::onhangup)
						flags |= DBUS_WATCH_HANGUP;

					if (events & MainLoop::onerror)
						flags |= DBUS_WATCH_ERROR;

					if (dbus_watch_handle(watch, flags) == FALSE) {
						cerr << "Error dbus_watch_handle() failed" << endl;
					}

					while (dbus_connection_get_dispatch_status(conn) == DBUS_DISPATCH_DATA_REMAINS)
						dbus_connection_dispatch(conn);

					return true;
				}
			);

			return TRUE;
		}

		static void remove_watch(DBusWatch *w, void *data) {
			MainLoop::getInstance().remove(w);
		}

		static void toggle_watch(DBusWatch *w, DBusConnection *conn) {

			if (dbus_watch_get_enabled(w))
				add_watch(w, conn);
			else
				remove_watch(w, conn);

		}

		static dbus_bool_t add_timeout(DBusTimeout *timeout, void *data) {

			if (!dbus_timeout_get_enabled(timeout))
				return TRUE;

			MainLoop::getInstance().insert(
				timeout,
				(unsigned long) dbus_timeout_get_interval(timeout),
				[timeout]() {
					dbus_timeout_handle(timeout);
					return true;
				}
			);

			return TRUE;
		}

		static void remove_timeout(DBusTimeout *t, void *data) {
			MainLoop::getInstance().remove(t);
		}

		static void toggle_timeout(DBusTimeout *t, void *data) {
			if (dbus_timeout_get_enabled(t))
				add_timeout(t, data);
			else
				remove_timeout(t, data);
		}

		Connection::Connection(DBusBusType type) {

			Error error;

			connct = dbus_bus_get(type,&error);
			error.test();

			dbus_connection_set_exit_on_disconnect(connct, false);

			if(!dbus_connection_set_watch_functions(
					connct,
					(DBusAddWatchFunction)		add_watch,
					(DBusRemoveWatchFunction)	remove_watch,
					(DBusWatchToggledFunction)	toggle_watch,
					connct,
					NULL
				)) {

				throw runtime_error("Error setting watch calls");
			}

			if (!dbus_connection_set_timeout_functions(
					connct,
					add_timeout,
					remove_timeout,
					toggle_timeout,
					connct,
					NULL
			)) {
				throw runtime_error("Error setting timeout calls");
			}

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


	}

 }


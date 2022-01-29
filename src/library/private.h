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

 #include <config.h>
 #include <udjat/tools/dbus.h>
 #include <iostream>

 using namespace std;
 using namespace Udjat;

 extern "C" {

	UDJAT_PRIVATE void handle_dispatch_status(DBusConnection *c, DBusDispatchStatus status, DBus::Connection *connection);

	UDJAT_PRIVATE dbus_bool_t add_watch(DBusWatch *w, DBus::Connection *connection);
	UDJAT_PRIVATE void remove_watch(DBusWatch *w, DBus::Connection *connection);
	UDJAT_PRIVATE void toggle_watch(DBusWatch *w, DBus::Connection *connection);

	UDJAT_PRIVATE dbus_bool_t add_timeout(DBusTimeout *t, DBus::Connection *connection);
	UDJAT_PRIVATE void remove_timeout(DBusTimeout *t, DBus::Connection *connection);
	UDJAT_PRIVATE void toggle_timeout(DBusTimeout *t, DBus::Connection *connection);

 }



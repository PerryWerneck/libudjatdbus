/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2015 Perry Werneck <perry.werneck@gmail.com>
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

 #include <config.h>
 #include "private.h"
 #include <udjat/tools/threadpool.h>

 using namespace Udjat;

/*---[ Implement ]----------------------------------------------------------------------------------*/

 void handle_dispatch_status(DBusConnection *c, DBusDispatchStatus UDJAT_UNUSED(status), DBus::Connection UDJAT_UNUSED(*connection)) {
	DBus::Connection::dispatch(c);
 }

 void DBus::Connection::dispatch(DBusConnection * connection) noexcept {
	lock_guard<recursive_mutex> lock(guard);
	while(connection && dbus_connection_get_dispatch_status(connection) == DBUS_DISPATCH_DATA_REMAINS) {
		dbus_connection_dispatch(connection);
	}
 }


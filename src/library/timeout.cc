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

 #include "private.h"
 #include <udjat/tools/mainloop.h>
 #include <unistd.h>

/*---[ Implement ]----------------------------------------------------------------------------------*/

 struct TimeoutContext {
	DBusConnection	* conn = nullptr;
	DBusTimeout		* timeout = nullptr;
	unsigned long	  ms = 0;

	constexpr TimeoutContext() {
	}

 };

 static bool handle_timeout(TimeoutContext *ctx) {
	dbus_timeout_handle(ctx->timeout);
	return TRUE;
 }

 dbus_bool_t add_timeout(DBusTimeout *t, DBus::Connection *connection) {

	if (!dbus_timeout_get_enabled(t))
		return TRUE;

	TimeoutContext *ctx = new TimeoutContext();

	ctx->conn		= connection->getConnection();
	ctx->timeout	= t;
	ctx->ms			= dbus_timeout_get_interval(t);

	dbus_timeout_set_data(t, ctx, NULL);

	if(dbus_timeout_get_enabled(ctx->timeout)) {

		Udjat::MainLoop::getInstance().insert(
			(void *) ctx,
			ctx->ms,
			[ctx]() {
				handle_timeout(ctx);
				return true;
			}
		);

	}

	return TRUE;
 }

 void remove_timeout(DBusTimeout *t, DBus::Connection UDJAT_UNUSED(*connection)) {

	TimeoutContext *ctx = (TimeoutContext *) dbus_timeout_get_data(t);

	if(ctx) {
		MainLoop::getInstance().remove(ctx);
		delete ctx;
	}
 }

 void toggle_timeout(DBusTimeout *t, DBus::Connection UDJAT_UNUSED(*connection)) {

	TimeoutContext *ctx = (TimeoutContext *) dbus_timeout_get_data(t);

	if(ctx) {

		MainLoop::getInstance().remove(ctx);
		delete ctx;

		if (dbus_timeout_get_enabled(ctx->timeout)) {

			ctx->ms	= dbus_timeout_get_interval(ctx->timeout);

			Udjat::MainLoop::getInstance().insert(
				(void *) ctx,
				ctx->ms,
				[ctx]() {
					handle_timeout(ctx);
					return true;
				}
			);

		}

	}

 }




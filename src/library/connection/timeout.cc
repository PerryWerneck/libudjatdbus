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
 #include <udjat/defs.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/timer.h>
 #include <private/mainloop.h>
 #include <unistd.h>

 class TimeoutContext : public Udjat::MainLoop::Timer {
 public:

	DBusConnection	* conn = nullptr;
	DBusTimeout		* timeout = nullptr;

	TimeoutContext() {
	}

 protected:
	void on_timer() override {
		dbus_timeout_handle(this->timeout);
	}

 };

 dbus_bool_t add_timeout(DBusTimeout *t, DBusConnection *connection) {

	TimeoutContext *ctx = new TimeoutContext();

	ctx->conn		= connection;
	ctx->timeout	= t;
	ctx->reset(dbus_timeout_get_interval(t));

	dbus_timeout_set_data(t, ctx, NULL);

	if(dbus_timeout_get_enabled(ctx->timeout)) {
		ctx->enable();
	}

	return TRUE;
 }

 void remove_timeout(DBusTimeout *t, DBusConnection *) {

	TimeoutContext *ctx = (TimeoutContext *) dbus_timeout_get_data(t);

	if(ctx) {
		delete ctx;
	}
 }

 void toggle_timeout(DBusTimeout *t, DBusConnection *) {

	TimeoutContext *ctx = (TimeoutContext *) dbus_timeout_get_data(t);

	if(ctx) {

		ctx->disable();

		if (dbus_timeout_get_enabled(ctx->timeout)) {

			ctx->reset(dbus_timeout_get_interval(ctx->timeout));
			ctx->enable();

		}

	}

 }



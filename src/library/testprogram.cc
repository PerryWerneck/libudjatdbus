/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2025 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tools/logger.h>
 #include <udjat/module/abstract.h>
 #include <udjat/tools/dbus.h>
 #include <udjat/tools/dbus/connection.h>
 #include <udjat/tools/dbus/message.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/dbus/service.h>
 #include <udjat/tools/dbus/signal.h>
 #include <string>
 
 using namespace Udjat;
 using namespace Udjat::DBus;
 using namespace std;

 #ifdef DEBUG 
 UDJAT_API int run_unit_test(const char *name) {

	Logger::String{"Running unit test: ",name}.info();

	SessionBus::getInstance().call_and_wait(
		"org.gnome.ScreenSaver",
		"/org/gnome/ScreenSaver",
		"org.gnome.ScreenSaver",
		"GetActiveTime",
		[](DBus::Message & message) {

			if(message) {

				unsigned int active;
				message.pop(active);

				debug("-------------------------> Got response active=",active);

			} else {

				debug("-------------------------> Error calling gnome");

			}

		});



	return 0;
 }
 #endif // DEBUG

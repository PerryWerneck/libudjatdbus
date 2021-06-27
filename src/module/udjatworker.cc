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

 /*
 #include "private.h"

 namespace Udjat {

	const Worker * getWorker(DBusMessage *message) {

		if(dbus_message_get_type(message) != DBUS_MESSAGE_TYPE_METHOD_CALL) {
			return nullptr;
		}

		static const char *interface = "br.eti.werneck." STRINGIZE_VALUE_OF(PRODUCT_NAME) ".";
		size_t intflen = strlen(interface);

		if(strncasecmp(dbus_message_get_interface(message),interface,intflen)) {
			return nullptr;
		}

		const char * worker = dbus_message_get_interface(message) + intflen;

#ifdef DEBUG
		cout << "Requested worker: '" << worker << "'" << endl;
#endif // DEBUG

		try {

			return Udjat::Worker::find(worker);

		} catch(...) {

			// Ignore exceptions.

		}

		return nullptr;

	}

 }
*/

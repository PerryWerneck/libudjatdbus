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

 #include <config.h>
 #include "private.h"

 namespace Udjat {

	void DBus::Connection::subscribe(void *id, const char *interface, const char *member, std::function<void(DBus::Message &message)> call) {
		lock_guard<recursive_mutex> lock(guard);
		cout << name << "\tSubscribing to " << interface << "." << member << endl;
		getInterface(interface).members.emplace_back(id,member,call);
	}

	void DBus::Connection::unsubscribe(void *id, const char *interface, const char *memberName) {

		lock_guard<recursive_mutex> lock(guard);

		if(getInterface(interface).unsubscribe(this,id,memberName)) {

			interfaces.remove_if([this](Interface &interface ){
				if(interface.empty()) {
					interface.remove_from(this);
					return true;
				}
				return false;
			});

		}

	}

	void DBus::Connection::unsubscribe(void *id) {

		lock_guard<recursive_mutex> lock(guard);
		interfaces.remove_if([this,id](Interface &interface){
			if(interface.unsubscribe(this,id)) {
				interface.remove_from(this);
				return true;
			}
			return false;
		});
	}

 }

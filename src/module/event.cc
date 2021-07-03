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

	DBus::Alert::Event::Event(const DBus::Alert &alert, int type, const Abstract::Agent &agent, const Abstract::State &state)
		: Udjat::Alert::Event(agent, state), message_type(type), path(alert.path), iface(alert.iface), member(alert.member) {

		state.expand(path);
		agent.expand(path);

		state.expand(iface);
		agent.expand(iface);

		state.expand(member);
		agent.expand(member);

		// Load arguments.
		for(auto argument = alert.begin(); argument != alert.end(); argument++) {

			string str{argument->value};
			state.expand(str);
			agent.expand(str);

			values.emplace_back(argument->type,str.c_str());

		}
	}

	const char * DBus::Alert::Event::getDescription() const {
		return iface.c_str();
	}

	void DBus::Alert::Event::alert(size_t current, size_t total) {

		if(message_type == DBUS_MESSAGE_TYPE_SIGNAL) {
			info("Emitting {}.{}{} ({}/{})",iface,member,path,current,total);
		} else {
			info("Calling {}.{}{} ({}/{})",iface,member,path,current,total);
		}

		Connection::getInstance().send(
			message_type,
			path.c_str(),
			iface.c_str(),
			member.c_str(),
			&values
		);

	}

 }


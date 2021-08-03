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
 #include <udjat/alert.h>
 #include <udjat/tools/xml.h>

 namespace Udjat {

	DBus::Signal::Factory::Factory() : Udjat::Factory("dbus-signal",&DBus::moduleinfo) {
	}

	bool DBus::Signal::Factory::parse(Abstract::Agent &parent, const pugi::xml_node &node) const {
		parent.push_back(make_shared<DBus::Signal>(node));
		return true;
	}

	void DBus::Signal::activate(const Abstract::Agent &agent, const Abstract::State &state) {

#ifdef DEBUG
		info("Signal '{}.{}' was activated",iface,member);
#endif // DEBUG

		Alert::activate(make_shared<DBus::Alert::Event>(*this,DBUS_MESSAGE_TYPE_SIGNAL,agent,state));

	}

 }


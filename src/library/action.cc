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
 #include <dbus/dbus.h>
 #include <stdexcept>
 #include <udjat/tools/actions/dbus.h>
 #include <udjat/tools/dbus/connection.h>

 using namespace std;

 namespace Udjat {

	DBus::Action::Action(const XML::Node &node) 
		: Udjat::Action{node},
		  message_type{DBUS_MESSAGE_TYPE_SIGNAL},	// TODO: Implement XML parsing for message type
		  bustype{BusTypeFactory(node)},
		  path{String{node,"dbus-path"}.as_quark()},
		  iface{String{node,"dbus-interface"}.as_quark()},
		  member{String{node,"dbus-member"}.as_quark()} {

		const char *props[] = {path,iface,member};
		const char *names[] = {"dbus-path","dbus-interface","dbus-member"};

		for(const auto prop : props) {
			if(!(prop && *prop)) {
				throw std::runtime_error(Logger::String{"Missing required attribute '",names[&prop - props],"'"});
			}
		}

		XML::load(node,"argument",arguments);

	}

	DBus::Action::Action(int m, DBusBusType b, const char *d, const char *p, const char *i, const char *mb)
		: Udjat::Action{"dbus"}, message_type{m}, bustype{b}, destination{d}, path{p}, iface{i}, member{mb} {
	}

 }

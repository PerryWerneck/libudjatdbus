/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
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

 /**
  * @brief Brief description of this source.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/dbus/member.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/logger.h>

 using namespace std;

 namespace Udjat {

	DBus::Member::Member(const char *name,const std::function<void(Message & message)> &c)
		: string{name}, callback{c}, type{DBUS_MESSAGE_TYPE_SIGNAL} {
		Logger::String{"Watching '",c_str(),"'"}.trace("d-bus");
	}

	DBus::Member::Member(const XML::Node &node,const std::function<void(Message & message)> &callback) : Member{String{node,"dbus-member"}.c_str(),callback} {
		Logger::String{"Watching '",c_str(),"'"}.trace("d-bus");
	}

	DBus::Member::~Member() {
		Logger::String{"Unwatching '",c_str(),"'"}.trace("d-bus");
	}

	bool DBus::Member::operator==(const char *name) const noexcept {
		return strcasecmp(name,c_str()) == 0;
	}

 }



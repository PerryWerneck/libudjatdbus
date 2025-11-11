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
 #include <udjat/tools/xml.h>
 #include <udjat/tools/actions/dbus.h>
 #include <udjat/tools/string.h>

 using namespace std;

 namespace Udjat {

	DBus::DBusType DBus::DBusTypeFactory(const XML::Node &node) {

		static const struct {
			const char *name;
			DBus::DBusType type;
		} dbus_types[] = {
			{"string", DBUS_TYPE_STRING},
			{"int16", DBUS_TYPE_INT16},
			{"uint16", DBUS_TYPE_UINT16},
			{"int32", DBUS_TYPE_INT32},
			{"uint32", DBUS_TYPE_UINT32},
			{"boolean", DBUS_TYPE_BOOLEAN},
			{"double", DBUS_TYPE_DOUBLE},
			{"objectpath", DBUS_TYPE_OBJECT_PATH},
			{"signature", DBUS_TYPE_SIGNATURE},
			{nullptr, DBUS_TYPE_INVALID}
		};

		String typestr{node,"type","string"};

		for(size_t i = 0; dbus_types[i].name != nullptr; i++) {
			if(strcasecmp(typestr.c_str(),dbus_types[i].name) == 0) {
				return dbus_types[i].type;
			}
		}

		throw runtime_error(String("Unknown D-Bus type: '",typestr,"'"));
		
	}

	DBus::Action::Argument::Argument(const XML::Node &node)
		: name{Abstract::Object::settings_from(node,"name","")},
		  tmplt{Abstract::Object::settings_from(node,"template","")},
		  type{DBusTypeFactory(node)} {
	}


 }

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
 #include <cstring>

 namespace Udjat {

	static int getType(const pugi::xml_node &node) {

		Udjat::Attribute attribute(node,"type",false);

		const char * type = attribute.as_string("string");

		static const struct {
			const char *name;
			int type;
		} types[] = {

			{ "byte",		DBUS_TYPE_BYTE		},
			{ "boolean",	DBUS_TYPE_BOOLEAN	},
			{ "int16",		DBUS_TYPE_INT16		},
			{ "uint16",		DBUS_TYPE_UINT16	},
			{ "int32",		DBUS_TYPE_INT32		},
			{ "uint32",		DBUS_TYPE_UINT32	},
			{ "int64",		DBUS_TYPE_INT64		},
			{ "uint64",		DBUS_TYPE_UINT64	},
			{ "double",		DBUS_TYPE_DOUBLE	},
			{ "string",		DBUS_TYPE_STRING	}

		};

		for(size_t ix = 0; ix < (sizeof(types)/sizeof(types[0])); ix++) {

			if(!strcasecmp(type,types[ix].name)) {
				return types[ix].type;
			}

		}

		throw runtime_error(string{"Unexpected type '"} + type + "'");

	}

	DBus::Alert::Argument::Argument(const pugi::xml_node &node) : type(getType(node)), value(Quark(node,"value","",false).c_str()) {

#ifdef DEBUG
		cout << ((char) this->type) << " - " << this->value << endl;
#endif // DEBUG

	}

 }


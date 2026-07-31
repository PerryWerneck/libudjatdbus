/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2026 Perry Werneck <perry.werneck@gmail.com>
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
 #include <private/datatable.h>
 #include <dbus/dbus.h>
 #include <udjat/tools/schema.h>
 #include <udjat/tools/variant.h>
 #include <private/response.h>	// value_factory
 #include <string>

 using namespace std;

 namespace Udjat {

	static const struct {
		Schema::Type schema;
		const char *dbus;

	} types[] = {
		{ Schema::String,		DBUS_TYPE_STRING_AS_STRING 	},
		{ Schema::Timestamp,	DBUS_TYPE_STRING_AS_STRING	},
		{ Schema::Signed,		DBUS_TYPE_INT32_AS_STRING	},
		{ Schema::Unsigned,		DBUS_TYPE_UINT32_AS_STRING	},
		{ Schema::Double,		DBUS_TYPE_DOUBLE_AS_STRING	},
		{ Schema::Float,		DBUS_TYPE_DOUBLE_AS_STRING	},
		{ Schema::Boolean,		DBUS_TYPE_BOOLEAN_AS_STRING	},
		{ Schema::Icon,			DBUS_TYPE_STRING_AS_STRING	},
		{ Schema::Url,			DBUS_TYPE_STRING_AS_STRING	},
		{ Schema::State,		DBUS_TYPE_STRING_AS_STRING	},
		{ Schema::Percent,		DBUS_TYPE_DOUBLE_AS_STRING	},
	};

	DBus::DataTable::DataTable(DBusMessage *request, const OutputSchema &s) : Udjat::DataTable{s}, reply{dbus_message_new_method_return(request)} {

		string signature;

		for(const auto &item : schema) {

			const char *dbus = nullptr;
			for(const auto &type : types) {
				if(type.schema == item.type()) {
					dbus = type.dbus;
					break;
				}
			}

			if(!dbus) {
				throw runtime_error("Unable to determine value type from schema");
			}

			signature += dbus;
		}

		debug("signature='",signature.c_str(),"'");

		dbus_message_iter_init_append(reply, &iter);

		if (!dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, signature.c_str(), &container)) {
        	dbus_message_unref(reply);
			throw runtime_error("Error opening container for response");
		}

	}

	DBus::DataTable::~DataTable() {
		dbus_message_iter_abandon_container_if_open(&iter, &container);
		dbus_message_unref(reply);
	}

	DBusMessage * DBus::DataTable::MessageFactory() {
		dbus_message_ref(reply);
		return reply;
	}

	void DBus::DataTable::push_back(const Schema::Item &schema, const Variant &value) {
		
		int type;
		DBusBasicValue dval;

		if(!value_factory(schema,type,value,dval)) {
			throw runtime_error("Unable to convert variant do dbus-value");
		}

		if(!dbus_message_append_args(reply, type, &value, DBUS_TYPE_INVALID)) {
			throw runtime_error("Failure adding value to table");
		}
	}

 }



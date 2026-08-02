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
 #include <private/tools.h>
 #include <dbus/dbus.h>
 #include <udjat/tools/schema.h>
 #include <udjat/tools/variant.h>
 #include <string>

 using namespace std;

 namespace Udjat {

	DBus::DataTable::DataTable(DBusMessage *message, const OutputSchema &s) : Udjat::DataTable{s}, request{message}, reply{dbus_message_new_method_return(message)} {

		dbus_message_ref(request);

		string signature = "(";

		for(const auto &item : schema) {
			signature += DBus::StringTypeFactory(item.type());
		}
		signature += ")";

		debug("signature='",signature.c_str(),"'");

		dbus_message_iter_init_append(reply, &iter);

		if (!dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, signature.c_str(), &array)) {
        	dbus_message_unref(reply);
			throw runtime_error("Error opening container for response");
		}

	}

	DBus::DataTable::~DataTable() {
		dbus_message_iter_abandon_container_if_open(&iter, &array);
		dbus_message_unref(reply);
		dbus_message_unref(request);
	}

	DBusMessage * DBus::DataTable::MessageFactory() {
		dbus_message_iter_close_container(&iter,&array);

		if(code == HTTP::Ok) {
			dbus_message_ref(reply);
			return reply;
		}

		debug("Sending error");
		return dbus_message_new_error(
			request,
			ErrorFactory(code),
			HTTP::Status::c_str()
		);

	}

	Udjat::DataTable & DBus::DataTable::push_back(const Value &row) {

		int type;
		DBusBasicValue dval;
		DBusMessageIter cols;

		if(!dbus_message_iter_open_container(&array, DBUS_TYPE_STRUCT, NULL, &cols)) {
			throw runtime_error("Unable to open response row");
		}

		try {

			for(const auto &item : schema) {

				if(!ValueFactory(row[item.name()],item,type,dval)) {
					throw runtime_error("Unable to convert variant do dbus-value");
				}

				if(!dbus_message_iter_append_basic(&cols, type, &dval)) {
					throw runtime_error("Failure adding value to table");
				}

			}

		} catch(...) {

			dbus_message_iter_close_container(&array,&cols);
			throw;

		}

		dbus_message_iter_close_container(&array,&cols);
		return *this;
		
	}

 }



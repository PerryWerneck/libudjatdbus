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
 #include <udjat/tools/response.h>
 #include <private/response.h>
 #include <udjat/tools/schema.h>
 #include <udjat/tools/interface.h>
 #include <dbus/dbus.h>
 #include <string>

 using namespace std;

 namespace Udjat {

	DBus::Response::Response(const OutputSchema &s) : schema{s}  {

	}

	DBus::Response::~Response() {

	}

	static bool dbus_value_factory(const Schema::Item &item, int &arg_type, const Udjat::Value &value, DBusBasicValue &dval) {

		switch(item.type()) {
		case Schema::String: 
		case Schema::Timestamp:
		case Schema::State: 
		case Schema::Icon:
		case Schema::Url:
		case Schema::Percent:
			arg_type = DBUS_TYPE_STRING;
			dval.str = (char *) value.c_str();
			break;

		case Schema::Signed:
			{
				int v;
				value.get(v);
				arg_type = DBUS_TYPE_INT32;
				dval.i32 = v;
			}
			break;

		case Schema::Unsigned:
			{
				unsigned int v;
				value.get(v);
				arg_type = DBUS_TYPE_UINT32;
				dval.u32 = v;
			}
			break;

		case Schema::Float: 
			{
				double v;
				value.get(v);
				arg_type = DBUS_TYPE_DOUBLE;
				dval.dbl = v;
			}
			break;

		case Schema::Boolean: 
			{
				bool v;
				value.get(v);
				arg_type = DBUS_TYPE_BOOLEAN;
				dval.bool_val = v;
			}
			break;

		default:
			return false;

		}

		return true;
	}

	DBusMessage * DBus::Response::MessageFactory(DBusMessage *message) {
	
		if(status_code() != HTTP::Ok) {

			// Failed, send message.
			return dbus_message_new_error(
				message,
				DBUS_ERROR_FAILED,
				(status.body.empty() ? status.message.c_str() : status.body.c_str())
			);

		}

		auto reply = dbus_message_new_method_return(message);

		// TODO: Check if response is an array and schema is enumerable.

		for(const auto &item : schema) {

			const char *name = item.name();

			if(!this->contains(name)) {
				dbus_message_unref(reply);
				return dbus_message_new_error(
					message,
					DBUS_ERROR_FAILED,
					Logger::Message{"Value for '{}' was not provided for the backend",name}.c_str()
				);
			}

			auto value = (*this)[name];

			int arg_type;
			DBusBasicValue dval;

			if(!dbus_value_factory(item,arg_type,value,dval)) {
				dbus_message_unref(reply);
				return dbus_message_new_error(
					message,
					DBUS_ERROR_FAILED,
					Logger::Message{"Invalid type for '{}'",name}.c_str()
				);
			}

			if(!dbus_message_append_args(reply, arg_type, &dval, DBUS_TYPE_INVALID)) {
				dbus_message_unref(reply);
				return dbus_message_new_error(
					message,
					DBUS_ERROR_FAILED,
					Logger::Message{"Unable to append '{}' to reply message",name}.c_str()
				);
			}
			
			value.clear();

		}

		return reply;
	}

 }



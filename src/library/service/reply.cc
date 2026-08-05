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
 #include <dbus/dbus.h>
 #include <udjat/tools/dbus/service.h>
 #include <udjat/tools/variant.h>
 #include <private/tools.h>

 using namespace std;

 namespace Udjat {

	static void add_value(DBusMessageIter &iter, const Variant &value) {

		DBusBasicValue dval;

		Variant::Type type = (Variant::Type) value; 
		switch(type) {
		case Variant::Object:
			{
				DBusMessageIter sub;
				if(!dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &sub)) {
					throw runtime_error("Unable to open response struct response");
				}

				try {

					value.for_each([&sub](const char *key, const Variant &value){

						DBusMessageIter entry;
						dbus_message_iter_open_container(
							&sub, 
							DBUS_TYPE_DICT_ENTRY, NULL, 
							&entry
						);

						dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
        
						DBusMessageIter variant;

						dbus_message_iter_open_container(
							&entry, 
							DBUS_TYPE_VARIANT, 
							DBus::StringTypeFactory((Variant::Type) value),
							&variant
						);
  
      					add_value(variant,value);

						dbus_message_iter_close_container(&entry, &variant);
						dbus_message_iter_close_container(&sub, &entry);
						return false;
					});

				} catch(...) {
					dbus_message_iter_abandon_container_if_open(&iter,&sub);
					throw;
				}
				
				dbus_message_iter_close_container(&iter,&sub);

			}
			break;

		case Variant::String: 
		case Variant::Timestamp:
		case Variant::State: 
		case Variant::Icon:
		case Variant::Url:
		case Variant::Fraction:
		case Variant::ObjectPath:
			dval.str = (char *) value.c_str();
			if(!dbus_message_iter_append_basic(&iter,DBUS_TYPE_STRING,&dval)) {
				throw runtime_error("Can't add value to d-bus iterator");
			}
			break;

		case Variant::Signed:
			{
				int v;
				value.get(v);
				dval.i32 = v;
				if(!dbus_message_iter_append_basic(&iter,DBUS_TYPE_INT32,&dval)) {
					throw runtime_error("Can't add value to d-bus iterator");
				}
			}
			break;

		case Variant::Unsigned:
			{
				unsigned int v;
				value.get(v);
				dval.u32 = v;
				if(!dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT32,&dval)) {
					throw runtime_error("Can't add value to d-bus iterator");
				}
			}
			break;

		case Variant::Real: 
			{
				double v;
				value.get(v);
				dval.dbl = v;
				if(!dbus_message_iter_append_basic(&iter,DBUS_TYPE_DOUBLE,&dval)) {
					throw runtime_error("Can't add value to d-bus iterator");
				}
			}
			break;

		case Variant::Boolean: 
			{
				bool v;
				value.get(v);
				dval.bool_val = v;
				if(!dbus_message_iter_append_basic(&iter,DBUS_TYPE_BOOLEAN,&dval)) {
					throw runtime_error("Can't add value to d-bus iterator");
				}
			}
			break;

		default:
			throw runtime_error(String{"Unable to find dbus-type for '",std::to_string(type),"'"});

		}

	}

	DBusMessage * DBus::Service::ReplyFactory(DBusMessage *request, const Variant &response) const noexcept {

		DBusMessageIter iter;

		auto reply = dbus_message_new_method_return(request);
		dbus_message_iter_init_append(reply, &iter);

		try {

			add_value(iter,response);

		} catch(const std::exception &e) {

			dbus_message_unref(reply);	
			return dbus_message_new_error(
						request,
						DBUS_ERROR_FAILED,
						e.what()
					);

		} catch(...) {

			dbus_message_unref(reply);	
			return dbus_message_new_error(
						request,
						DBUS_ERROR_FAILED,
						"Unexpected error building reply"
					);

		}

		return reply;

	}

 }

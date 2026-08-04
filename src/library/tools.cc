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
 #include <private/tools.h>
 #include <udjat/tools/http/method.h>
 #include <udjat/tools/http/statuscodes.h>
 #include <udjat/tools/schema.h>
 #include <stdexcept>

 using namespace std;

 namespace Udjat {

	static const struct {
		HTTP::Method http;
		const char *dbus;
	} http_methods[] = {
		{ HTTP::Get,	"get"		},
		{ HTTP::Head,	"state"		},
		{ HTTP::Post,	"insert"	},
		{ HTTP::Put,	"replace"	},
		{ HTTP::Delete,	"delete"	},
		{ HTTP::Patch,	"update"	},
	};

	UDJAT_PRIVATE const char * DBus::MethodNameFactory(const HTTP::Method http) noexcept {
		for(const auto &method : http_methods) {
			if(method.http == http) {
				return method.dbus;
			}
		}
		return nullptr;
	}

	UDJAT_PRIVATE bool DBus::for_each(const std::function<bool(const HTTP::Method http, const char *dbus)> &callback) noexcept {
		for(const auto &method : http_methods) {
			if(callback(method.http,method.dbus)) {
				return true;
			}
		}
		return false;
	}

	UDJAT_PRIVATE HTTP::Method HTTP::MethodFactory(DBusMessage *message) {
		const char *member = dbus_message_get_member(message);
		for(const auto &method : http_methods) {
			if(!strcasecmp(method.dbus,member)) {
				return method.http;
			}
		}
		Logger::String{"Invalid method call: '",dbus_message_get_interface(message),".",member,"'"}.error();
		return HTTP::UnknownMethod;
	}

	static const struct {
		Schema::Type schema;
		Variant::Type variant;
		int type;
		const char *str;
	} type_mappings[] = {
		{ Schema::ObjectPath,	Variant::ObjectPath,	DBUS_TYPE_STRING,	DBUS_TYPE_STRING_AS_STRING	},
		{ Schema::String,		Variant::String, 		DBUS_TYPE_STRING,	DBUS_TYPE_STRING_AS_STRING 	},
		{ Schema::Timestamp,	Variant::Timestamp, 	DBUS_TYPE_STRING,	DBUS_TYPE_STRING_AS_STRING	},
		{ Schema::Signed,		Variant::Signed, 		DBUS_TYPE_INT32,	DBUS_TYPE_INT32_AS_STRING	},
		{ Schema::Unsigned,		Variant::Unsigned, 		DBUS_TYPE_UINT32,	DBUS_TYPE_UINT32_AS_STRING	},
		{ Schema::Double,		Variant::Real,	 		DBUS_TYPE_DOUBLE,	DBUS_TYPE_DOUBLE_AS_STRING	},
		{ Schema::Float,		Variant::Real,	 		DBUS_TYPE_DOUBLE,	DBUS_TYPE_DOUBLE_AS_STRING	},
		{ Schema::Boolean,		Variant::Boolean, 		DBUS_TYPE_BOOLEAN,	DBUS_TYPE_BOOLEAN_AS_STRING	},
		{ Schema::Icon,			Variant::Icon,	 		DBUS_TYPE_STRING,	DBUS_TYPE_STRING_AS_STRING	},
		{ Schema::Url,			Variant::Url,	 		DBUS_TYPE_STRING,	DBUS_TYPE_STRING_AS_STRING	},
		{ Schema::State,		Variant::State, 		DBUS_TYPE_STRING,	DBUS_TYPE_STRING_AS_STRING	},
		{ Schema::Percent,		Variant::Fraction, 		DBUS_TYPE_DOUBLE,	DBUS_TYPE_DOUBLE_AS_STRING	},
	};

	UDJAT_PRIVATE const char * DBus::StringTypeFactory(const Schema::Type schema_type) noexcept {
		for(const auto &type : type_mappings) {
			if(type.schema == schema_type) {
				return type.str;
			}
		}
		return DBUS_TYPE_STRING_AS_STRING;
	}

	UDJAT_PRIVATE const char * DBus::StringTypeFactory(const Variant::Type variant_type) noexcept {
		for(const auto &type : type_mappings) {
			if(type.variant == variant_type) {
				return type.str;
			}
		}
		return DBUS_TYPE_STRING_AS_STRING;
	}

	UDJAT_PRIVATE bool DBus::ValueFactory(const Udjat::Variant &value, const Schema::Item &item, int &arg_type, DBusBasicValue &dval) noexcept {

		switch(item.type()) {
		case Schema::String: 
		case Schema::Timestamp:
		case Schema::State: 
		case Schema::Icon:
		case Schema::Url:
		case Schema::Percent:
		case Schema::ObjectPath:
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
			Logger::String{"Unable to find dbus-type for '",std::to_string(item.type()),"'"}.error();
			return false;

		}

		return true;
	}

	static const struct {
		HTTP::StatusCode code;
		const char *dbus;
	} http_status_codes[] = {

		{ HTTP::BadRequest,			DBUS_ERROR_INVALID_ARGS },
		{ HTTP::UnAuthenticated,	DBUS_ERROR_ACCESS_DENIED },
		{ HTTP::Forbidden,			DBUS_ERROR_ACCESS_DENIED },
		{ HTTP::NotFound,			DBUS_ERROR_FILE_NOT_FOUND },
		{ HTTP::MethodNotAllowed,	DBUS_ERROR_UNKNOWN_METHOD },
		{ HTTP::ProxyAuthRequired,	DBUS_ERROR_ACCESS_DENIED },
		{ HTTP::RequestTimeout,		DBUS_ERROR_TIMEOUT },
		{ HTTP::NotImplemented,		DBUS_ERROR_NOT_SUPPORTED },

	};

	UDJAT_PRIVATE const char * DBus::ErrorFactory(const HTTP::StatusCode code) {
		for(const auto &status : http_status_codes) {
			if(status.code == code) {
				return status.dbus;
			}
		}
		return DBUS_ERROR_FAILED;
	}

 }


 


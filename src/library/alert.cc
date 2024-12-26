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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/abstract/object.h>
 #include <udjat/tools/dbus/connection.h>
 #include <udjat/alert.h>
 #include <udjat/alert/d-bus.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/string.h>
 #include <dbus/dbus.h>
 #include <stdexcept>

 using namespace std;

 namespace Udjat {

	static int TypeFactory(const XML::Node &node) {

		static const struct {
			int type;
			const char *name;
		} typenames[] = {
			{
				DBUS_TYPE_BYTE,
				"byte",
			},
			{
				DBUS_TYPE_BOOLEAN,
				"boolean",
			},
			{
				DBUS_TYPE_INT16,
				"int16",
			},
			{
				DBUS_TYPE_UINT16,
				"uint16",
			},
			{
				DBUS_TYPE_INT32,
				"int32",
			},
			{
				DBUS_TYPE_UINT32,
				"uint32",
			},
			{
				DBUS_TYPE_INT64,
				"int64",
			},
			{
				DBUS_TYPE_UINT64,
				"uint64",
			},
			{
				DBUS_TYPE_DOUBLE,
				"double",
			},
			{
				DBUS_TYPE_STRING,
				"string",
			},
			{
				DBUS_TYPE_OBJECT_PATH,
				"object-path",
			},
			{
				DBUS_TYPE_SIGNATURE,
				"signature",
			},
			{
				DBUS_TYPE_UNIX_FD,
				"unix-fd",
			},
			{
				DBUS_TYPE_ARRAY,
				"array",
			},
			{
				DBUS_TYPE_VARIANT,
				"variant",
			},
			{
				DBUS_TYPE_STRUCT,
				"struct",
			},
			{
				DBUS_TYPE_DICT_ENTRY,
				"dict-entry",
			},
		};

		String name{node,"type"};
		if(name.empty()) {
			throw runtime_error(Logger::String{"Required argument 'type' is missing or empty on <",node.name(),">"});
		}

		for(size_t ix = 0; ix < (sizeof(typenames)/sizeof(typenames[0]));ix++) {
			if(!strcasecmp(name.c_str(),typenames[ix].name)) {
				return typenames[ix].type;
			}
		}

		throw runtime_error("Unsupported argument type");
	}

	DBus::Alert::Input::Input(const XML::Node &node) : name{String{node,"name"}.as_quark()}, type{TypeFactory(node)} {

		memset(&dbval,0,sizeof(dbval));
		switch(type) {
		case DBUS_TYPE_STRING:
			dbval.str = (char *) String{node,"value"}.as_quark();
			break;

		case DBUS_TYPE_BOOLEAN:
			dbval.bool_val = node.attribute("value").as_bool();
			break;

		case DBUS_TYPE_INT16:
			dbval.i16 = (int16_t) node.attribute("value").as_int();
			break;
			
		case DBUS_TYPE_UINT16:
			dbval.u16 = (uint16_t) node.attribute("value").as_uint();
			break;
			
		case DBUS_TYPE_INT32:
			dbval.i32 = (int32_t) node.attribute("value").as_int();
			break;
			
		case DBUS_TYPE_UINT32:
			dbval.u16 = (uint32_t) node.attribute("value").as_uint();
			break;
			
		case DBUS_TYPE_INT64:
			dbval.i64 = (int64_t) node.attribute("value").as_ullong();
			break;
			
		case DBUS_TYPE_UINT64:
			dbval.u64 = (uint64_t) node.attribute("value").as_uint();
			break;
			
		case DBUS_TYPE_DOUBLE:
			dbval.dbl = node.attribute("value").as_double();
			break;
						
		default:
			throw runtime_error("Unsupported argument type");

		}

	}

	DBus::Alert::Alert(const XML::Node &node) 
		: Udjat::Alert{node}, 
			message_type{dbus_message_type_from_string(String{node,"message-type","signal"}.c_str())},
			bustype{BusTypeFactory(node)},
			path{String{node,"path"}.as_quark()}, 
			iface{String{node,"interface"}.as_quark()}, 
			member{String{node,"member"}.as_quark()} {

		if(!(message_type == DBUS_MESSAGE_TYPE_SIGNAL || message_type == DBUS_MESSAGE_TYPE_METHOD_CALL)) {
			throw runtime_error("Invalid message type");
		}

		// Get inputs
		for(auto child = node.child("argument"); child; child = child.next_sibling("argument")) {
			inputs.emplace_back(child);
		}

	}

	DBus::Alert::~Alert() {
	}

	void DBus::Alert::reset(bool active) noexcept {
		if(!active) {
			arguments.clear();
			activation.iface.clear();
			activation.member.clear();
			activation.path.clear();
		}
		::Udjat::Alert::reset(active);
	}

	bool DBus::Alert::activate() noexcept {
		if(active()) {
			return false;
		}

		prepare();

		if(!inputs.empty()) {
			Logger::String{"Empty activation, using default arguments"}.trace(name());
			for(const auto &input : inputs) {
				arguments.emplace_back(input.type,input.dbval);
			}
		}

		return ::Udjat::Alert::activate();
	}

	void DBus::Alert::prepare() {
		arguments.clear();
		activation.iface = iface;
		activation.member = member;
		activation.path = path;
	}

	bool DBus::Alert::activate(const Abstract::Object &object) noexcept {

		if(active()) {
			return false;
		}

		prepare();
		activation.iface.expand(object);
		activation.member.expand(object);
		activation.path.expand(object);

		for(const auto &input : inputs) {

			string value;

			if(object.getProperty(input.name,value)) {

				Argument arg{input.type,value.c_str()};

				switch(arg.type) {
				case DBUS_TYPE_STRING:
					arg.dbval.str = (char *) arg.value.c_str();
					break;

				case DBUS_TYPE_BOOLEAN:
					arg.dbval.bool_val = (atoi(arg.value.c_str()) != 0);
					break;

				case DBUS_TYPE_INT16:
					arg.dbval.i16 = (int16_t) atoi(arg.value.c_str());
					break;
					
				case DBUS_TYPE_UINT16:
					arg.dbval.u16 = (uint16_t) atoi(arg.value.c_str());
					break;
					
				case DBUS_TYPE_INT32:
					arg.dbval.i32 = (int32_t) atoi(arg.value.c_str());
					break;
					
				case DBUS_TYPE_UINT32:
					arg.dbval.u16 = (uint32_t) atoi(arg.value.c_str());
					break;
					
				case DBUS_TYPE_INT64:
					arg.dbval.i64 = (int64_t) atoll(arg.value.c_str());
					break;
					
				case DBUS_TYPE_UINT64:
					arg.dbval.u64 = (uint64_t) atoll(arg.value.c_str());
					break;
					
				case DBUS_TYPE_DOUBLE:
					arg.dbval.dbl = (double) atof(arg.value.c_str());
					break;
								
				default:
					Logger::String{"Unsupported argument type"}.error(name());
					return false;

				}

				arguments.push_back(arg);

			} else {

				Logger::String{"Using default value for argument '",input.name,"'"}.trace(name());
				arguments.emplace_back(input.type,input.dbval);

			}

		}

		return ::Udjat::Alert::activate();
	}

	int DBus::Alert::emit() {

		DBusMessage *message{dbus_message_new(message_type)};

		Logger::String{
			"Emitting dbus://",
			activation.iface.c_str(),
			" ",
			activation.path.c_str(),
			" ",
			activation.member.c_str()
		}.trace(name());

		dbus_message_set_interface(message,activation.iface.c_str());
		dbus_message_set_path(message,activation.path.c_str());
		dbus_message_set_member(message,activation.member.c_str());

		DBusMessageIter iter;
		dbus_message_iter_init_append(message,&iter);
		for(const auto &argument : arguments) {
			dbus_message_iter_append_basic(&iter, argument.type, &argument.dbval);
		}

		try {

			Connection::getInstance(bustype).call(message);
			
		} catch(...) {
	
			dbus_message_unref(message);
			throw;

		}

		dbus_message_unref(message);

		return 0;

	}

 }

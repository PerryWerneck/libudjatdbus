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
 #include <udjat/tools/dbus/interface.h>
 #include <udjat/tools/dbus/member.h>
 #include <udjat/tools/dbus/connection.h>
 #include <udjat/tools/dbus/emitter.h>
 #include <udjat/alert/d-bus.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/string.h>
 #include <dbus/dbus.h>
 #include <stdexcept>
 #include <udjat/tools/singleton.h>

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

	DBus::Emitter::Output::Output(const XML::Node &node) : name{String{node,"name"}.as_quark()}, type{TypeFactory(node)} {

		if(!(name && *name)) {
			throw runtime_error("Required attribute 'name' is missing or empty");
		}

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

	static Singleton::Container<DBus::Emitter> emitters;

	DBus::Emitter::Emitter(const XML::Node &node) 
		: message_type{dbus_message_type_from_string(String{node,"message-type","signal"}.c_str())},
			bustype{BusTypeFactory(node)},
			path{String{node,"dbus-path"}.as_quark()}, 
			iface{Abstract::DBus::Interface::NameFactory(node).as_quark()}, 
			member{DBus::Member::NameFactory(node).as_quark()} {

		if(!(path && *path)) {
			throw runtime_error("Required attribute 'dbus-path' is missing or empty");
		}

		if(!(message_type == DBUS_MESSAGE_TYPE_SIGNAL || message_type == DBUS_MESSAGE_TYPE_METHOD_CALL)) {
			throw runtime_error("Invalid message type");
		}

		// Get inputs
		for(auto child = node.child("argument"); child; child = child.next_sibling("argument")) {
			outputs.emplace_back(child);
		}

		emitters.getInstance().push_back(this);

	}

	DBus::Emitter::~Emitter() {
		emitters.getInstance().remove(this);
	} 

	bool DBus::Emitter::for_each(const std::function<bool(const DBus::Emitter &emitter)> &method) {
		return emitters.getInstance().for_each(method);
	}

	void DBus::Emitter::validate() const {

		if(activation.iface.empty()) {
			throw logic_error("Unable to emit d-bus message with empty interface name");
		}

		if(activation.path.empty()) {
			throw logic_error("Unable to emit d-bus message with empty path");
		}

	}

	void DBus::Emitter::clear() noexcept {
		arguments.clear();
		activation.iface.clear();
		activation.member.clear();
		activation.path.clear();
	}

	void DBus::Emitter::prepare() {
		arguments.clear();
		activation.iface = iface;
		activation.member = member;
		activation.path = path;

		validate();

		// Load default inputs.
		for(const auto &output : outputs) {
			arguments.emplace_back(output.type,output.dbval);
		}
	}


	void DBus::Emitter::introspect(std::stringstream &xmldata) const {

/*
<signal name="StateChanged">
  <arg name="state" type="i"/>
  <arg name="error" type="s"/>
</signal>stringstream
*/
		xmldata << "<signal name=\"" << member << "\">";

		for(const auto &output : outputs) {
			xmldata << "<arg name=\"" << output.name << "\" type=\"" << ((char) output.type) << "\"/>";
		}

		xmldata << "</signal>";

	}

	void DBus::Emitter::prepare(const Abstract::Object &object) {

		arguments.clear();
		activation.iface = iface;
		activation.member = member;
		activation.path = path;

		activation.iface.expand(object);
		activation.member.expand(object);
		activation.path.expand(object);

		validate();

		for(const auto &output : outputs) {

			string value;

			if(object.getProperty(output.name,value)) {

				Argument arg{output.type,value.c_str()};

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
					throw runtime_error("Unsupported argument type");

				}

				arguments.push_back(arg);

			} else {

				Logger::String{"Using default value for argument '",output.name,"'"}.trace();
				arguments.emplace_back(output.type,output.dbval);

			}

		}

	}

	void DBus::Emitter::send() {

		validate();

		DBusMessage *message{dbus_message_new(message_type)};

		Logger::String{
			"Emitting ",
			dbus_message_type_to_string(message_type)," ",
			activation.iface.c_str()," ",
			activation.path.c_str()," ",
			activation.member.c_str()
		}.trace();

		debug("Interface name will bet set to '",activation.iface.c_str(),"'");
		dbus_message_set_interface(message,activation.iface.c_str());

		debug("Path will bet set to '",activation.path.c_str(),"'");
		dbus_message_set_path(message,activation.path.c_str());

		debug("Member will bet set to '",activation.member.c_str(),"'");
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

	}

 }

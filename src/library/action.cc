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
 #include <dbus/dbus.h>
 #include <stdexcept>
 #include <udjat/tools/actions/dbus.h>
 #include <udjat/tools/dbus/connection.h>
 #include <udjat/tools/dbus/exception.h>
 #include <udjat/tools/dbus/message.h>
 #include <udjat/tools/memory.h>
 #include <udjat/tools/logger.h>

 using namespace std;

 namespace Udjat {

	std::shared_ptr<Udjat::Action> DBus::Action::Factory::ActionFactory(const XML::Node &node) const {
		return std::make_shared<DBus::Action>(node);
	}

	DBus::Action::Action(const XML::Node &node) 
		: Udjat::Action{node},
		  message_type{DBUS_MESSAGE_TYPE_SIGNAL},	// TODO: Implement XML parsing for message type
		  bustype{BusTypeFactory(node)},
		  path{String{node,"dbus-path"}.as_quark()},
		  iface{String{node,"dbus-interface"}.as_quark()},
		  member{String{node,"dbus-member"}.as_quark()} {

		const char *props[] = {path,iface,member};
		const char *names[] = {"dbus-path","dbus-interface","dbus-member"};

		for(const auto prop : props) {
			if(!(prop && *prop)) {
				throw std::runtime_error(Logger::String{"Missing required attribute '",names[&prop - props],"'"});
			}
		}

		XML::load(node,"argument",arguments);

	}

	DBus::Action::Action(int m, DBusBusType b, const char *d, const char *p, const char *i, const char *mb)
		: Udjat::Action{"dbus"}, message_type{m}, bustype{b}, destination{d}, path{p}, iface{i}, member{mb} {
	}

	DBus::Action::~Action() {
	}	

	std::shared_ptr<DBusMessage> DBus::Action::MessageFactory(const std::vector<String> &vals) {

		std::shared_ptr<DBusMessage> message =
			make_handle(
				dbus_message_new(message_type),
				dbus_message_unref
			);

		if(message_type != DBUS_MESSAGE_TYPE_METHOD_CALL) {
			dbus_message_set_destination(message.get(),destination);
		}
		
		DBusMessageIter iter;
		dbus_message_iter_init_append(message.get(), &iter);
		for(size_t ix = 0; ix < arguments.size(); ix++) {

			DBusBasicValue val;
			memset(&val,0,sizeof(val));

			switch(arguments[ix].type) {
			case DBUS_TYPE_STRING:
				val.str = (char *) vals[ix].c_str();
				break;

			case DBUS_TYPE_INT16:
				val.i16 = atoi(vals[ix].c_str());
				break;

			case DBUS_TYPE_UINT16:
				val.u16 = static_cast<uint16_t>(atoi(vals[ix].c_str()));
				break;

			case DBUS_TYPE_INT32:
				val.i32 = atoi(vals[ix].c_str());
				break;

			case DBUS_TYPE_UINT32:
				val.u32 = static_cast<uint32_t>(atoi(vals[ix].c_str()));
				break;

			case DBUS_TYPE_INT64:
				val.i64 = static_cast<int64_t>(atoll(vals[ix].c_str()));
				break;

			case DBUS_TYPE_UINT64:
				val.u64 = static_cast<uint64_t>(atoll(vals[ix].c_str()));
				break;

			case DBUS_TYPE_DOUBLE:
				val.dbl = atof(vals[ix].c_str());
				break;

			case DBUS_TYPE_BOOLEAN:
				val.bool_val = atoi(vals[ix].c_str()) != 0;
				break;

			default:
				throw std::system_error(ENOTSUP,system_category(),"Unsupported D-Bus argument type");
			}

			if(!dbus_message_iter_append_basic(&iter,arguments[ix].type,&val)) {
				throw runtime_error("Can't add value to d-bus iterator");
			}

		}

		return message;
	}

	int DBus::Action::call(Udjat::Request &request, Udjat::Response &response, bool except) {

		try {

			std::vector<String> vals;

			message = MessageFactory(vals);
			dbus_message_set_interface(message.get(),iface);
			dbus_message_set_path(message.get(),path);
			dbus_message_set_member(message.get(),member);

			// TODO: Check for introspection to get property names.

			// It's a method call, we need to wait for a reply.


			/*
			DBusMessageIter iter;
			dbus_message_iter_init_append(message.get(),&iter);
			for(size_t ix = 0; ix < arguments.size(); ix++) {
				dbus_message_iter_append_basic(&iter, arguments[ix].type, &values[ix]);
			}

			DBusError error;
			dbus_error_init(&error);

			debug("Calling and waiting...");
			
			DBusMessage * rsp =
				dbus_connection_send_with_reply_and_block(
					Connection::getInstance(bustype).get(),
					message.get(),
					DBUS_TIMEOUT_USE_DEFAULT,
					&error
				);

			if(dbus_error_is_set(&error)) {
				if(rsp) {
					dbus_message_unref(rsp);
				}
				throw runtime_error(Logger::String{"D-Bus call error: ",error.name," - ",error.message});
			}

			if(!rsp) {
				throw runtime_error("No response received from D-Bus call");
			}

			DBus::Message msg{rsp};
			dbus_message_unref(rsp);

			// TODO: Use introspection data to build proper response.
			msg.for_each([&response](const Udjat::Value &value) {
				response.append(value);
				return false;
			});
			*/

		} catch(const system_error &e) {
	
			if(except) {
				throw;
			}
			return e.code().value();

		} catch(const std::exception &e) {
			
			if(except) {
				throw;
			}
			return -1;

		}

		return 0;

	}

	/// Update arguments from object.
	/// @param object The object with new argument values.
	void DBus::Action::call(const Udjat::Abstract::Object &object) {

		std::vector<String> vals;
		for(const auto &arg : arguments) {
			String str{arg.tmplt};
			str.expand(object,true);
			vals.push_back(str);
		}

		message = MessageFactory(vals);
		dbus_message_set_interface(message.get(),String{iface}.expand(object,true).c_str());
		dbus_message_set_path(message.get(),String{path}.expand(object,true).c_str());
		dbus_message_set_member(message.get(),String{member}.expand(object,true).c_str());

	}

	/// @brief Call action using already set parameters.
	/// @param except if true will launch exceptions on errors.
	/// @return 0 if ok, error code otherwise.
	int DBus::Action::call(bool except) {

		try {

			if(!message) {
				message = MessageFactory();
				dbus_message_set_interface(message.get(),iface);
				dbus_message_set_path(message.get(),path);
				dbus_message_set_member(message.get(),member);
			}
			
		} catch(const system_error &e) {
			if(except) {
				throw;
			}
			Logger::String{"Action failed: ",e.what()}.error(name());
			return e.code().value();
		} catch(const exception &e) {
			if(except) {
				throw;
			}
			Logger::String{"Action failed: ",e.what()}.error(name());
			return -1;
		} catch(...) {
			if(except) {
				throw;
			}
			Logger::String{"Action failed: Unknown error"}.error(name());
			return -1;
		}

		return 0;

	}

	/*
	void DBus::Action::load(const std::vector<Udjat::String> &vals) {

		// Get lengths.
		size_t valsize = sizeof(DBusBasicValue) * arguments.size();
		size_t strsize = 0;

		for(size_t ix = 0; ix < arguments.size(); ix++) {
			if(arguments[ix].type == DBUS_TYPE_STRING) {
				// +1 for null terminator.
				strsize += vals[ix].length() + 1;
			}
		}

		// Allocate values.
		uint8_t *str_block = (uint8_t *) malloc(valsize + strsize);
		memset(str_block,0,valsize + strsize);

		char *ptr = (char *) (str_block + valsize);

		debug("Loading D-Bus action arguments for action '",name(),"'");
		unload();
		values = reinterpret_cast<DBusBasicValue *>(str_block);

		for(size_t ix = 0; ix < arguments.size(); ix++) {

			switch(arguments[ix].type) {
			case DBUS_TYPE_STRING:
				debug("Loading string argument '",arguments[ix].name,"' with value '",vals[ix],"'");
				strcpy(ptr,vals[ix].c_str());
				values[ix].str = ptr;
				ptr += vals[ix].length() + 1;
				break;

			case DBUS_TYPE_INT16:
				values[ix].i16 = atoi(vals[ix].c_str());
				break;

			case DBUS_TYPE_UINT16:
				values[ix].u16 = static_cast<uint16_t>(atoi(vals[ix].c_str()));
				break;

			case DBUS_TYPE_INT32:
				values[ix].i32 = atoi(vals[ix].c_str());
				break;

			case DBUS_TYPE_UINT32:
				values[ix].u32 = static_cast<uint32_t>(atoi(vals[ix].c_str()));
				break;

			case DBUS_TYPE_INT64:
				values[ix].i64 = static_cast<int64_t>(atoll(vals[ix].c_str()));
				break;

			case DBUS_TYPE_UINT64:
				values[ix].u64 = static_cast<uint64_t>(atoll(vals[ix].c_str()));
				break;

			case DBUS_TYPE_DOUBLE:
				values[ix].dbl = atof(vals[ix].c_str());
				break;

			case DBUS_TYPE_BOOLEAN:
				values[ix].bool_val = atoi(vals[ix].c_str()) != 0;
				break;

			default:
				unload();
				throw std::system_error(ENOTSUP,system_category(),"Unsupported D-Bus argument type");
			}
		}

	}
	*/

	/*
	void DBus::Action::load(const Udjat::Request &request) {

		debug("Loading D-Bus action arguments from request for action '",name(),"'");
		std::vector<String> vals;
		for(const auto &arg : arguments) {
			String str{arg.tmplt};
			str.expand([&request](const char *key, std::string &value){
				return request.getProperty(key,value);
			},true);
			vals.push_back(str);
		}

		load(vals);

	}
	*/

	/*
	/// @brief Load argument values from object.
	/// @param object The object with argument values.
	void DBus::Action::load(const Udjat::Abstract::Object &object) {

		debug("Loading D-Bus action arguments from object for action '",name(),"'");
		std::vector<String> vals;
		for(const auto &arg : arguments) {
			String str{arg.tmplt};
			debug("Argument '",arg.name,"' template before expand: '",str.c_str(),"'");
			str.expand(object,true);
			vals.push_back(str);
		}

		load(vals);
	}
	*/

	/*
	void DBus::Action::exec() {

		if(!values) {
			throw std::runtime_error("No argument values loaded");
		}

		auto message = make_handle(dbus_message_new(message_type),dbus_message_unref);

		debug("Interface name will bet set to '",iface,"'");
		dbus_message_set_interface(message.get(),iface);

		debug("Path will bet set to '",path,"'");
		dbus_message_set_path(message.get(),path);

		debug("Member will bet set to '",member,"'");
		dbus_message_set_member(message.get(),member);

		DBusMessageIter iter;
		dbus_message_iter_init_append(message.get(),&iter);
		for(size_t ix = 0; ix < arguments.size(); ix++) {
			dbus_message_iter_append_basic(&iter, arguments[ix].type, &values[ix]);
		}

		switch(message_type) {
		case DBUS_MESSAGE_TYPE_SIGNAL:
			Logger::String{
				"Emitting ",
				dbus_message_type_to_string(message_type)," ",
				iface,".",member," on path ",
				path
			}.trace(name());
			Connection::getInstance(bustype).call(message.get());
			break;
		
		default:
			throw std::system_error(ENOTSUP,system_category(),Logger::String{"Unsupported D-Bus message type: ",dbus_message_type_to_string(message_type)});

		}

	}
	*/

 }

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
 #include <udjat/tools/actions/dbus.h>
 #include <udjat/tools/dbus/connection.h>
 #include <stdexcept>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/xml.h>	
 #include <udjat/tools/memory.h>
 #include <sstream>

 using namespace std;

 namespace Udjat {

	int DBus::Action::MessageTypeFactory(const XML::Node &node, const char *def, const char *attrname) {

		static const struct 
		{
			int type;
			const char *name;
		} typenames[] = {
			{
				DBUS_MESSAGE_TYPE_METHOD_CALL,
				"method_call",
			},
			{
				DBUS_MESSAGE_TYPE_METHOD_CALL,
				"call",
			},
			{
				DBUS_MESSAGE_TYPE_ERROR,
				"error",
			},
			{
				DBUS_MESSAGE_TYPE_SIGNAL,
				"signal",
			},
		};

		String attrvalue = String{node,attrname,def};

		for(const auto &tn : typenames) {
			if(!strcasecmp(attrvalue.c_str(),tn.name)) {
				return tn.type;
			}
		}

		throw runtime_error(String{"Unknown or invalid D-Bus message type: '",attrvalue,"'"});

	}

	DBus::Action::Action(const XML::Node &node)
		: Udjat::Action{node},
		  message_type{MessageTypeFactory(node)},
		  bustype{BusTypeFactory(node)},
		  service{String{node,"dbus-service"}.as_quark()},
		  path{String{node,"dbus-path"}.as_quark()},
		  interface{String{node,"dbus-interface"}.as_quark()},
		  member{String{node,"dbus-member"}.as_quark()} {

		if(!(service && *service) && message_type != DBUS_MESSAGE_TYPE_SIGNAL) {
			throw runtime_error("Required attribute 'service' is missing or empty");
		}

		if(!(path && *path)) {
			throw runtime_error("Required attribute 'path' is missing or empty");
		}

		if(!(interface && *interface)) {
			throw runtime_error("Required attribute 'interface' is missing or empty");
		}

		if(!(member && *member)) {
			throw runtime_error("Required attribute 'member' is missing or empty");
		}

		XML::load(node,"argument",arguments);

	}

	bool DBus::Action::activate(const Udjat::Abstract::Object &object) noexcept {

		debug("--- Activating D-Bus action '",interface,".",member,"' ---");
		
		debug("Setting up ",arguments.size()," arguments");
		for(auto &argument : arguments) {
			DBusBasicValue dbval;
			argument.set(object,dbval);
		}

		return Activatable::activate(object);
	}

	std::shared_ptr<DBusMessage> DBus::Action::MessageFactory() const {

		auto message = make_handle(dbus_message_new(message_type),dbus_message_unref);

		debug("Interface name will bet set to '",interface,"'");
		dbus_message_set_interface(message.get(),interface);

		debug("Path will bet set to '",path,"'");
		dbus_message_set_path(message.get(),path);

		debug("Member will bet set to '",member,"'");
		dbus_message_set_member(message.get(),member);

		return message;
	}

	void DBus::Action::get_arguments(std::shared_ptr<DBusMessage> message, const Udjat::Request &request) {
		DBusMessageIter iter;
		dbus_message_iter_init_append(message.get(),&iter);

		debug("Setting up ",arguments.size()," arguments");
		for(auto &argument : arguments) {
			DBusBasicValue dbval;
			dbus_message_iter_append_basic(&iter, argument.dbus_type(), argument.set(request,dbval));
		}

	}

	void DBus::Action::get_arguments(std::shared_ptr<DBusMessage> message, const Udjat::Abstract::Object &object) {
		DBusMessageIter iter;
		dbus_message_iter_init_append(message.get(),&iter);

		debug("Setting up ",arguments.size()," arguments");
		for(auto &argument : arguments) {
			DBusBasicValue dbval;
			dbus_message_iter_append_basic(&iter, argument.dbus_type(), argument.set(object,dbval));
		}

	}

	int DBus::Action::call(bool except) {

		Logger::String{
			"Calling ",
			dbus_message_type_to_string(message_type)," ",
			interface," ",
			path," ",
			member
		}.trace();

		auto message = MessageFactory();
		DBusMessageIter iter;
		dbus_message_iter_init_append(message.get(),&iter);

		debug("Setting up ",arguments.size()," arguments");
		for(auto &argument : arguments) {
			DBusBasicValue dbval;
			dbus_message_iter_append_basic(&iter, argument.dbus_type(), argument.get(dbval));
		}
		
		try {

			if(message_type == DBUS_MESSAGE_TYPE_SIGNAL) {
				Connection::getInstance(bustype).call(message.get());
				return 0;
			} else {
				throw std::system_error(ENOTSUP, std::system_category(),"D-Bus method calls are not supported yet");
			}

		} catch(const std::system_error &e) {
			if(except) {
				throw;
			}
			Logger::String{e.what()}.error();
			return e.code().value();

		} catch(const std::exception &e) {
			if(except) {
				throw;
			}
			Logger::String{e.what()}.error();

		} catch(...) {
			if(except) {
				throw;
			}
			Logger::String{"Unexpected error emitting d-bus signal"}.error();
		}

		return -1;

	}

	int DBus::Action::call(Udjat::Request &request, Udjat::Response &response, bool except) {

		Logger::String{
			"Emitting ",
			dbus_message_type_to_string(message_type)," ",
			interface," ",
			path," ",
			member
		}.trace();

		auto message = MessageFactory();
		get_arguments(message,request);

		// Got properties, set action as active.
		Activatable::activate();

		try {

			if(message_type == DBUS_MESSAGE_TYPE_SIGNAL) {
				Connection::getInstance(bustype).call(message.get());
				return 0;
			} else {
				throw std::system_error(ENOTSUP, std::system_category(),"D-Bus method calls are not supported yet");
			}

		} catch(const std::system_error &e) {
			if(except) {
				throw;
			}
			Logger::String{e.what()}.error();
			return e.code().value();

		} catch(const std::exception &e) {
			if(except) {
				throw;
			}
			Logger::String{e.what()}.error();

		} catch(...) {
			if(except) {
				throw;
			}
			Logger::String{"Unexpected error emitting d-bus signal"}.error();
		}

		return -1;

	}

	void DBus::Action::introspect(std::stringstream &xmldata) const {

		if(message_type == DBUS_MESSAGE_TYPE_SIGNAL) {

/*
<signal name="StateChanged">
  <arg name="state" type="i"/>
  <arg name="error" type="s"/>
</signal>stringstream
*/

			xmldata << "<signal name=\"" << member << "\">";
			for(const auto &argument : arguments) {
				xmldata << "<arg name=\"" << argument.c_str() << "\" type=\"" << ((char) argument.dbus_type()) << "\"/>";
			}
			xmldata << "</signal>";

		}

	}

 }

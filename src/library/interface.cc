/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tools/dbus/interface.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/logger.h>
 #include <stdexcept>

 using namespace std;

 namespace Udjat {

	Abstract::DBus::Interface::Interface(const char *n, const char *t) : std::string{n}, type{t} {
	}

	/// @brief Scan XML definition for interface name.
	String Abstract::DBus::Interface::NameFactory(const XML::Node &node) {

		// First check for specific names
		{
			static const char *attrnames[] = {
				"dbus-interface-name",
				"dbus-interface",
				"interface-name"
			};

			for(const char *attrname : attrnames) {
				String name{node,attrname};
				if(!name.empty()) {
					if(name[0] == '.') {
						return String{PRODUCT_ID,name.c_str()};
					}
					return name;
				}
			}
		}

		// Then the generic ones
		{
			static const char *attrnames[] = {
				"dbus-name",
				"name"
			};

			for(const char *attrname : attrnames) {
				String name{node,attrname};
				if(!name.empty()) {
					Logger::String{"Required interface name attribute is missing on <",node.name(),">, using default"}.warning();
					return String{PRODUCT_ID,".",name.c_str()};
				}
			}
		}

		throw runtime_error("The d-bus interface name is empty or invalid");
	}

	Abstract::DBus::Interface::Interface(const XML::Node &node, const char *t) : std::string{NameFactory(node)}, type{t} {
	}

	Abstract::DBus::Interface::~Interface() {
	}

	DBus::Interface::~Interface() {
	}

	const std::string Abstract::DBus::Interface::rule() const {
		return String{"type='",type,"',interface='",c_str(),"'"};
	}

	bool Abstract::DBus::Interface::operator==(const char *intf) const noexcept {
		return strcasecmp(intf,c_str()) == 0;
	}

	Udjat::DBus::Member & DBus::Interface::push_back(const XML::Node &node,const std::function<bool(Udjat::DBus::Message & message)> &callback) {
		return members.emplace_back(node,callback);
	}

	Udjat::DBus::Member & DBus::Interface::emplace_back(const char *member, const std::function<bool(Udjat::DBus::Message & message)> &callback) {
		return members.emplace_back(member,callback);
	}

	void DBus::Interface::remove(const Udjat::DBus::Member &member) {
		members.remove_if([&member](Udjat::DBus::Member &m){
			return &m == &member;
		});
	}

	DBusHandlerResult DBus::Interface::filter(DBusMessage *message) const {

		int type = dbus_message_get_type(message);
		const char *name = dbus_message_get_member(message);

		for(auto &member : members) {

			if(member == type && member == name) {
				Udjat::DBus::Message msg(message);
				member.call(msg);
			}

		}

		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}


 }



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

 namespace Udjat {

	DBus::Interface::Interface(const char *n) : std::string{n}, type{"signal"} {
	}

	DBus::Interface::~Interface() {
	}

	const std::string DBus::Interface::rule() const {
		return String{"type='",type,"',interface='",c_str(),"'"};
	}

	bool DBus::Interface::operator==(const char *intf) const noexcept {
		return false;
	}

 }


 /*
 #include <udjat/tools/dbus.h>
 #include <udjat/worker.h>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	DBus::Connection::Interface & DBus::Connection::getInterface(const char *name) {

		std::lock_guard<recursive_mutex> lock(guard);

		for(auto interface=interfaces.begin(); interface != interfaces.end(); interface++) {
			if(!interface->name.compare(name)) {
				return *interface;
			}
		}

		// Not found, create a new filter.
		DBusError error;
		dbus_error_init(&error);

		cout << this->name << "\tConnecting to " << name << endl;

		dbus_bus_add_match(connection,Interface::getMatch(name).c_str(), &error);
		dbus_connection_flush(connection);

		if (dbus_error_is_set(&error)) {
			std::string message(error.message);
			dbus_error_free(&error);
			throw std::runtime_error(message);
		}

		// Inclui na lista e retorna.
		interfaces.emplace_back(name);
		return interfaces.back();

	}

	/// @brief Unsubscribe by id.
	bool DBus::Connection::Interface::unsubscribe(const DBus::Connection *connection, void *id) {
		members.remove_if([this,connection,id](Listener &listener){
			if(listener.id == id) {
				cout << connection->c_str() << "\tUnsubscribing from " << this->name << "." << listener.name << endl;
				return true;
			}
			return false;
		});
		return members.empty();
	}

	/// @brief Unsubscribe by memberName.
	bool DBus::Connection::Interface::unsubscribe(const DBus::Connection *connection, void *id, const char *memberName) {
		members.remove_if([id,this,memberName,connection](Listener &listener){
			if(listener.id == id && strcmp(listener.name.c_str(),memberName) == 0) {
				cout << connection->c_str() << "\tUnsubscribing from " << this->name << "." << listener.name << endl;
				return true;
			}
			return false;
		});
		return members.empty();
	}

	std::string DBus::Connection::Interface::getMatch(const char *name) {
		std::string match{"type='signal',interface='"};
		match += name;
		match += "'";
		return match;
	}

	void DBus::Connection::Interface::remove_from(const DBus::Connection * connection) noexcept {

		cout << connection->c_str() << "\tDisconnecting from " << name << endl;

		DBusError error;
		dbus_error_init(&error);

		{
			lock_guard<recursive_mutex> lock(guard);
			dbus_bus_remove_match(connection->getConnection(),getMatch().c_str(), &error);
		}

		if(dbus_error_is_set(&error)) {
			cerr << name << "\tError '" << error.message << "' removing interface '" << name << "'" << std::endl;
			dbus_error_free(&error);
		}

		//dbus_connection_flush(connection); // NOTE: Check if this is really necessary.

 	}

 }
 */


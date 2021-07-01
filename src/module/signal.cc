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

 #include "private.h"
 #include <udjat/alert.h>
 #include <udjat/tools/xml.h>

 namespace Udjat {

	DBus::Signal::Signal(const pugi::xml_node &node) : Udjat::Alert(node) {

		path = Udjat::Attribute(node,"path").as_quark("/").c_str();
		iface = Udjat::Attribute(node,"interface").as_quark("br.eti.werneck." STRINGIZE_VALUE_OF(PRODUCT_NAME)).c_str();
		name = Udjat::Attribute(node,"name").as_quark("alert").c_str();

#ifdef DEBUG
		info("Signal '{}.{}' was created",iface,name);
#endif // DEBUG
	}

	DBus::Signal::~Signal() {

	}

	void DBus::Signal::activate(const Abstract::Agent &agent, const Abstract::State &state) {

		class Event : public Alert::Event {
		private:
			string path;
			string iface;
			string name;

		public:
			Event(const string &p, const string &i, const string &n) : path(p), iface(i), name(n) {
			}

			const char * getDescription() const override {
				return iface.c_str();
			}

			void alert(size_t current, size_t total) override {

				info("Emitting {}.{} ({}/{})",iface,name,current,total);

				DBusMessage *msg = dbus_message_new_signal(path.c_str(),iface.c_str(),name.c_str());
				Connection::getInstance().send(msg);

			}

		};

		string path = this->path;
		string iface = this->iface;
		string name = this->name;

		state.expand(path);
		agent.expand(path);

		state.expand(iface);
		agent.expand(iface);

		state.expand(name);
		agent.expand(name);

#ifdef DEBUG
		info("Signal '{}.{}' was activated",iface,name);
#endif // DEBUG

		Alert::activate(make_shared<Event>(path,iface,name));

	}


 }


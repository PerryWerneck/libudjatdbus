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

		path = Quark(node,"path","${agent.path}").c_str();
		iface = Quark(node,"interface","br.eti.werneck." STRINGIZE_VALUE_OF(PRODUCT_NAME) ".agent").c_str();
		member = Quark(node,"member","${level}").c_str();

#ifdef DEBUG
		info("Signal created iface='{}' name='{}'",iface,member);
#endif // DEBUG
	}

	DBus::Signal::~Signal() {

	}

	void DBus::Signal::activate(const Abstract::Agent &agent, const Abstract::State &state) {

		class Event : public Alert::Event {
		private:
			string path;
			string iface;
			string member;

		public:
			Event(const string &p, const string &i, const string &m) : path(p), iface(i), member(m) {
			}

			const char * getDescription() const override {
				return iface.c_str();
			}

			void alert(size_t current, size_t total) override {

				info("Emitting {} {} ({}/{})",iface,member,current,total);
#ifdef DEBUG
				info("Path='{}'",path);
#endif // DEBUG

				DBusMessage *msg = dbus_message_new_signal(path.c_str(),iface.c_str(),member.c_str());
				Connection::getInstance().send(msg);

			}

		};

		string path = this->path;
		string iface = this->iface;
		string member = this->member;

		state.expand(path);
		agent.expand(path);

		state.expand(iface);
		agent.expand(iface);

		state.expand(member);
		agent.expand(member);

#ifdef DEBUG
		info("Signal '{}.{}' was activated",iface,member);
#endif // DEBUG

		Alert::activate(make_shared<Event>(path,iface,member));

	}


 }


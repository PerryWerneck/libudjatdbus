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

	DBus::Signal::Factory::Factory() : Udjat::Factory("dbus-signal",&DBus::moduleinfo) {
	}

	void DBus::Signal::Factory::parse(Abstract::Agent &parent, const pugi::xml_node &node) const {
		parent.push_back(make_shared<DBus::Signal>(node));
	}

	void DBus::Signal::activate(const Abstract::Agent &agent, const Abstract::State &state) {

		class Event : public Alert::Event {
		private:
			string path;
			string iface;
			string member;

			std::vector<DBus::Value> values;

		public:
			Event(const Signal *signal, const Abstract::Agent &agent, const Abstract::State &state)
				: path(signal->path), iface(signal->iface), member(signal->member) {

				state.expand(path);
				agent.expand(path);

				state.expand(iface);
				agent.expand(iface);

				state.expand(member);
				agent.expand(member);

				// Load arguments.
				for(auto argument = signal->begin(); argument != signal->end(); argument++) {

					string str{argument->value};
					state.expand(str);
					agent.expand(str);

					values.emplace_back(argument->type,str.c_str());

				}

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

#ifdef DEBUG
		info("Signal '{}.{}' was activated",iface,member);
#endif // DEBUG

		Alert::activate(make_shared<Event>(this,agent,state));

	}

 }


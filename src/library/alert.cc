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
 #include <udjat/tools/object.h>
 #include <udjat/alert/activation.h>
 #include <dbus/dbus-protocol.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/dbus/signal.h>
 #include <string>
 #include <udjat/alert/d-bus.h>

 using namespace std;

 namespace Udjat {

	DBus::Alert::Argument::Argument(const Abstract::Object UDJAT_UNUSED(&parent), const char UDJAT_UNUSED(*group), const pugi::xml_node &node) : value{node,"value"} {

		static const struct {
			int type;
			const char *name;
		} types[] = {
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

		const char *type = node.attribute("type").as_string("string");

		for(size_t ix = 0; ix < (sizeof(types)/sizeof(types[0]));ix++) {

			if(!strcasecmp(type,types[ix].name)) {
				this->type = types[ix].type;
				break;
			}

		}

		if(this->type == DBUS_TYPE_INVALID) {
			throw system_error(EINVAL,system_category(),(string{"Unknown argument type: "} + type));
		}


	}

	DBus::Alert::Alert(const Abstract::Object &parent, const pugi::xml_node &node) : Abstract::Alert(node) {

		const char *group = node.attribute("settings-from").as_string("alert-defaults");

		debug("Creating d-bus alert '",name(),"'");

		path = getAttribute(node,group,"dbus-path","${agent.path}");
		if(!*path) {
			throw system_error(EINVAL,system_category(),"Required attribute <dbus-path> is missing or empty");
		}

		iface = getAttribute(node,group,"dbus-interface","");
		if(!*iface) {
			throw system_error(EINVAL,system_category(),"Required attribute <dbus-interface> is missing or empty");
		}

		member = getAttribute(node,group,"dbus-member","");
		if(!*member) {
			throw system_error(EINVAL,system_category(),"Required attribute <dbus-member> is missing or empty");
		}

		// Get bus type
		{
			static DBusBusType types[] = {
				DBUS_BUS_SESSION,
				DBUS_BUS_SYSTEM,
				DBUS_BUS_STARTER
			};

			size_t type = String(node,"dbus-bus-type","starter").select("session","system","starter",NULL);

			if(type >= (sizeof(types)/sizeof(types[0]))) {
				throw runtime_error("Invalid bus type");
			}

			this->bustype = types[type];
		}

		for(auto argument = node.child("argument"); argument; argument = argument.next_sibling("argument")) {
			arguments.emplace_back(parent,group,argument);
		}

	}

	DBus::Alert::~Alert() {
	}

	std::shared_ptr<Udjat::Alert::Activation> DBus::Alert::ActivationFactory() const {

		class Activation : public Udjat::Alert::Activation {
		private:
			String path;
			String iface;
			String member;
			DBusBusType bustype;

			std::vector<Alert::Argument> arguments;

		protected:

			void emit() override {

				DBus::Signal signal{
					iface.expand(true,true).c_str(),
					member.expand(true,true).c_str(),
					path.expand(true,true).c_str()
				};

				debug("---> Emitting D-Bus alert ",iface.c_str()," ",member.c_str(),"/",path.c_str());

				for(Alert::Argument &argument : arguments) {
					switch(argument.type) {
						case DBUS_TYPE_BOOLEAN:
							signal.push_back((bool) (stoi(argument.value) != 0));
							break;

						case DBUS_TYPE_INT16:
							signal.push_back((int16_t) stoi(argument.value));
							break;

						case DBUS_TYPE_UINT16:
							signal.push_back((uint16_t) stoul(argument.value));
							break;

						case DBUS_TYPE_INT32:
							signal.push_back((int32_t) stoi(argument.value));
							break;

						case DBUS_TYPE_UINT32:
							signal.push_back((uint32_t) stoul(argument.value));
							break;

						case DBUS_TYPE_INT64:
							signal.push_back((int64_t) stoll(argument.value));
							break;

						case DBUS_TYPE_UINT64:
							signal.push_back((uint64_t) stoull(argument.value));
							break;

						case DBUS_TYPE_STRING:
							signal.push_back(argument.value);
							break;

						// case DBUS_TYPE_DOUBLE:
						// case DBUS_TYPE_BYTE:
						// case DBUS_TYPE_OBJECT_PATH:
						// case DBUS_TYPE_SIGNATURE:
						// case DBUS_TYPE_UNIX_FD:
						// case DBUS_TYPE_ARRAY:
						// case DBUS_TYPE_VARIANT:
						// case DBUS_TYPE_STRUCT:
						// case DBUS_TYPE_DICT_ENTRY:
						default:
							throw system_error(EINVAL,system_category(), "Unsupported value type");

					}
					debug("Argument: ",argument.value);
				}

				switch(bustype) {
				case DBUS_BUS_SESSION:
					signal.session();
					break;

				case DBUS_BUS_SYSTEM:
					signal.system();
					break;

				case DBUS_BUS_STARTER:
					signal.starter();
					break;

				default:
					throw system_error(EINVAL,system_category(), "Invalid bus type");
				}

			}

		public:
			Activation(const DBus::Alert *alert) : Udjat::Alert::Activation(alert), path(alert->path), iface(alert->iface), member(alert->member), bustype(alert->bus()) {

				for(const Alert::Argument &argument : alert->args()) {
					arguments.push_back(argument);
				}

			}

			Udjat::Alert::Activation & set(const Abstract::Object &object) override {

				path.expand(object);
				iface.expand(object);
				member.expand(object);

				for(Alert::Argument &argument : arguments) {
					argument.value.expand(object);
				}

				return *this;
			}

			Udjat::Alert::Activation & set(const std::function<bool(const char *key, std::string &value)> &expander) override {
				path.expand(expander);
				iface.expand(expander);
				member.expand(expander);

				for(Alert::Argument &argument : arguments) {
					argument.value.expand(expander);
				}

				return *this;
			}

		};

		debug("Activating D-Bus alert");

		return make_shared<Activation>(this);

	}

 }


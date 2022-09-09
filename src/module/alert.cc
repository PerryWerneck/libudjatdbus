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
 #include "private.h"

 using namespace std;

 namespace Udjat {

	DBus::Alert::Argument::Argument(const Abstract::Object UDJAT_UNUSED(&parent), const char UDJAT_UNUSED(*group), const pugi::xml_node &node) {

		static const struct {
			int type;
			const char *name;
			const std::function<void(DBusBasicValue &value, String &str)> call;
		} types[] = {
			{
				DBUS_TYPE_BYTE,
				"byte",
				[](DBusBasicValue &value, String &str) {
					value.byt = str[0];
				}
			},
			{
				DBUS_TYPE_BOOLEAN,
				"boolean",
				[](DBusBasicValue &value, String &str) {
					value.bool_val = str.as_bool();
				}
			},
			{
				DBUS_TYPE_INT16,
				"int16",
				[](DBusBasicValue &value, String &str) {
					value.i16 = (dbus_int16_t) atoi(str.c_str());
				}
			},
			{
				DBUS_TYPE_UINT16,
				"uint16",
				[](DBusBasicValue &value, String &str) {
					value.u16 = (dbus_uint16_t) atoi(str.c_str());
				}
			},
			{
				DBUS_TYPE_INT32,
				"int32",
				[](DBusBasicValue &value, String &str) {
					value.u32 = (dbus_int32_t) atoi(str.c_str());
				}
			},
			{
				DBUS_TYPE_UINT32,
				"uint32",
				[](DBusBasicValue &value, String &str) {
					value.u32 = (dbus_uint32_t) atoi(str.c_str());
				}
			},
			{
				DBUS_TYPE_INT64,
				"int64",
				[](DBusBasicValue &value, String &str) {
					value.i64 = (dbus_int64_t) atoll(str.c_str());
				}
			},
			{
				DBUS_TYPE_UINT64,
				"uint64",
				[](DBusBasicValue &value, String &str) {
					value.u64 = (dbus_uint64_t) atoll(str.c_str());
				}
			},
			{
				DBUS_TYPE_DOUBLE,
				"double",
				[](DBusBasicValue &value, String &str) {
					value.dbl = (double) atof(str.c_str());
				}
			},
			{
				DBUS_TYPE_STRING,
				"string",
				[](DBusBasicValue &value, String &str) {
					value.str = (char *) Quark(str).c_str();
				}
			},
			{
				DBUS_TYPE_OBJECT_PATH,
				"object-path",
				[](DBusBasicValue UDJAT_UNUSED(&value), String UDJAT_UNUSED(&str)) {
					throw system_error(ENOTSUP,system_category(), "Unsupported argument type");
				}
			},
			{
				DBUS_TYPE_SIGNATURE,
				"signature",
				[](DBusBasicValue UDJAT_UNUSED(&value), String UDJAT_UNUSED(&str)) {
					throw system_error(ENOTSUP,system_category(), "Unsupported argument type");
				}
			},
			{
				DBUS_TYPE_UNIX_FD,
				"unix-fd",
				[](DBusBasicValue UDJAT_UNUSED(&value), String UDJAT_UNUSED(&str)) {
					throw system_error(ENOTSUP,system_category(), "Unsupported argument type");
				}
			},
			{
				DBUS_TYPE_ARRAY,
				"array",
				[](DBusBasicValue UDJAT_UNUSED(&value), String UDJAT_UNUSED(&str)) {
					throw system_error(ENOTSUP,system_category(), "Unsupported argument type");
				}
			},
			{
				DBUS_TYPE_VARIANT,
				"variant",
				[](DBusBasicValue UDJAT_UNUSED(&value), String UDJAT_UNUSED(&str)) {
					throw system_error(ENOTSUP,system_category(), "Unsupported argument type");
				}
			},
			{
				DBUS_TYPE_STRUCT,
				"struct",
				[](DBusBasicValue UDJAT_UNUSED(&value), String UDJAT_UNUSED(&str)) {
					throw system_error(ENOTSUP,system_category(), "Unsupported argument type");
				}
			},
			{
				DBUS_TYPE_DICT_ENTRY,
				"dict-entry",
				[](DBusBasicValue UDJAT_UNUSED(&value), String UDJAT_UNUSED(&str)) {
					throw system_error(ENOTSUP,system_category(), "Unsupported argument type");
				}
			},
		};

		const char *type = node.attribute("type").as_string("string");
		String str{node,"value"};

		for(size_t ix = 0; ix < (sizeof(types)/sizeof(types[0]));ix++) {

			if(!strcasecmp(type,types[ix].name)) {
				this->type = types[ix].type;
				types[ix].call(this->value,str);
				break;
			}

		}

		if(this->type == DBUS_TYPE_INVALID) {
			throw system_error(EINVAL,system_category(),(string{"Unknown argument type: "} + type));
		}


	}

	DBus::Alert::Alert(const Abstract::Object &parent, const pugi::xml_node &node) : Abstract::Alert(node) {

		const char *group = node.attribute("settings-from").as_string("alert-defaults");

		path = getAttribute(node,group,"dbus-path");
		iface = getAttribute(node,group,"dbus-interface");
		member = getAttribute(node,group,"dbus-member");

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

			std::vector<String> strings;

		protected:

			void emit() override {
				throw system_error(ENOTSUP,system_category(), "D-Bus alert support is incomplete");
			}

		public:
			Activation(const DBus::Alert *alert) : Udjat::Alert::Activation(alert), path(alert->path), iface(alert->iface), member(alert->member) {

				// TODO: Parse arguments.

			}

			Udjat::Alert::Activation & set(const Abstract::Object &object) override {
				path.expand(object);
				iface.expand(object);
				member.expand(object);
				return *this;
			}

			Udjat::Alert::Activation & expand(const std::function<bool(const char *key, std::string &value)> &expander) override {
				path.expand(expander);
				iface.expand(expander);
				member.expand(expander);
				return *this;
			}

		};


		return make_shared<Activation>(this);
	}

 }


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
 #include <udjat/dbus.h>
 #include <cstring>
 #include <string>

 namespace Udjat {

	DBus::Value::Value() {
		type = DBUS_TYPE_INVALID;
		memset(&value,0,sizeof(value));
	}

	DBus::Value::Value(int type, const char *str) : DBus::Value::Value() {

		reset();
		this->type = type;

		switch(type) {
		case DBUS_TYPE_BYTE:
			value.byt = str[0];
			break;

		case DBUS_TYPE_BOOLEAN:
			value.bool_val = (std::stoi(str) != 0);
			break;

		case DBUS_TYPE_INT16:
			value.i16 = std::stoi(str);
			break;

		case DBUS_TYPE_UINT16:
			value.u16 = std::stoi(str);
			break;

		case DBUS_TYPE_INT32:
			value.i32 = std::stol(str);
			break;

		case DBUS_TYPE_UINT32:
			value.u32 = std::stoul(str);
			break;

		case DBUS_TYPE_INT64:
			value.i64 = std::stoll(str);
			break;

		case DBUS_TYPE_UINT64:
			value.u64 = std::stoull(str);
			break;

		case DBUS_TYPE_DOUBLE:
			value.dbl = stod(str);
			break;

		case DBUS_TYPE_STRING:
			this->set(str);
			break;

		default:
			throw runtime_error("Unexpected type id");
		}

	}

	DBus::Value::~Value() {
		reset(Value::Type::Undefined);
	}

	bool DBus::Value::isNull() const {
		return type == DBUS_TYPE_INVALID;
	}

	Udjat::Value & DBus::Value::append(const Type UDJAT_UNUSED(unused)) {

		if(type != DBUS_TYPE_ARRAY) {
			reset();
			type = DBUS_TYPE_ARRAY;
		}

		Value * rc = new Value();
		children[std::to_string((int) children.size()).c_str()] = rc;
		return *rc;

	}

	Udjat::Value & DBus::Value::set(const Udjat::Value &value) {
		throw system_error(ENOTSUP,system_category(),"Cant set value");
	}

	Udjat::Value & DBus::Value::operator[](const char *name) {

		if(type != DBUS_TYPE_DICT_ENTRY) {
			reset();
			type = DBUS_TYPE_DICT_ENTRY;
		}

		auto search = children.find(name);
		if(search != children.end()) {
			return *search->second;
		}

		Value * rc = new DBus::Value();

		children[name] = rc;

		return *rc;

	}

	Udjat::Value & DBus::Value::reset(const Udjat::Value::Type UDJAT_UNUSED(unused)) {

		if(type == DBUS_TYPE_STRING && value.str) {
			free(value.str);
			value.str = NULL;
		}

		type = DBUS_TYPE_INVALID;
		memset(&value,0,sizeof(value));

		// cleanup children.
		for(auto child : children) {
			delete child.second;
		}

		children.clear();

		return *this;
	}

	Udjat::Value & DBus::Value::set(const TimeStamp value) {

		reset(Value::Type::Undefined);
		this->type = DBUS_TYPE_STRING;

		if(value) {
			this->value.str = strdup(value.to_string("%Y-%m-%d %H:%M:%S").c_str());
		} else {
			this->value.str = strdup("");
		}

		return *this;
	}

	Udjat::Value & DBus::Value::set(const char *value, const Type UDJAT_UNUSED(type)) {
		reset(Value::Type::Undefined);
		this->type = DBUS_TYPE_STRING;
		this->value.str = strdup(value);
		return *this;
	}

	Udjat::Value & DBus::Value::set(const short value) {
		reset(Value::Type::Undefined);
		this->type = DBUS_TYPE_INT16;
		this->value.i16 = value;
		return *this;
	}

	Udjat::Value & DBus::Value::set(const unsigned short value) {
		reset(Value::Type::Undefined);
		this->type = DBUS_TYPE_UINT16;
		this->value.u16 = value;
		return *this;
	}

	Udjat::Value & DBus::Value::set(const int value) {
		reset(Value::Type::Undefined);
		this->type = DBUS_TYPE_INT32;
		this->value.i32 = value;
		return *this;
	}

	Udjat::Value & DBus::Value::set(const unsigned int value) {
		reset(Value::Type::Undefined);
		this->type = DBUS_TYPE_UINT32;
		this->value.u32 = value;
		return *this;
	}

	Udjat::Value & DBus::Value::set(const long value) {
		reset(Value::Type::Undefined);
		this->type = DBUS_TYPE_INT64;
		this->value.i64 = value;
		return *this;
	}

	Udjat::Value & DBus::Value::set(const unsigned long value) {
		reset(Value::Type::Undefined);
		this->type = DBUS_TYPE_UINT64;
		this->value.u64 = value;
		return *this;
	}

	Udjat::Value & DBus::Value::set(const bool value) {
		reset(Value::Type::Undefined);
		this->type = DBUS_TYPE_BOOLEAN;
		this->value.bool_val = value;
		return *this;
	}

	Udjat::Value & DBus::Value::set(const float value) {
		reset(Value::Type::Undefined);
		this->type = DBUS_TYPE_DOUBLE;
		this->value.dbl = value;
		return *this;
	}

	Udjat::Value & DBus::Value::set(const double value) {
		reset(Value::Type::Undefined);
		this->type = DBUS_TYPE_DOUBLE;
		this->value.dbl = value;
		return *this;
	}

	string DBus::Value::getArraySignature() const noexcept {

		string str{"("};

		for(auto child : this->children) {

			if(child.second->noSignature()) {
				continue;
			}

			char v[2];
			v[0] = (char) child.second->type;
			v[1] = 0;
			str += v;
		}


		str += ")";
		return str;

	}


	void DBus::Value::get(DBusMessageIter *iter) {

		switch(this->type) {
		case DBUS_TYPE_INVALID:
			return;

		case DBUS_TYPE_ARRAY:
			{
				DBusMessageIter subIter;

				if(!this->children.empty()) {

					string signature = this->children.begin()->second->getArraySignature();

					if(dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, signature.c_str(), &subIter)) {

						for(auto row : this->children) {

							DBusMessageIter aIter;

							if(dbus_message_iter_open_container(&subIter, DBUS_TYPE_STRUCT, NULL, &aIter)) {

								const char *ptr = signature.c_str() + 1;
								for(auto child : row.second->children) {

									if(child.second->noSignature() || !*ptr) {
										continue;
									}

									if(*ptr != ((char) child.second->type)) {

										cerr << "DBus\tUnexpected signature. Got '"
												<< ((char) child.second->type)
												<< "' while expecting for '"
												<< *ptr << "'" << endl;
										continue;
									}

									ptr++;
									child.second->get(&aIter);

								}

								dbus_message_iter_close_container(&subIter,&aIter);

							}
						}

						dbus_message_iter_close_container(iter, &subIter);
					}
				}

				/*
				if(dbus_message_iter_open_container(iter, DBUS_TYPE_STRUCT, NULL, &subIter)) {

					for(auto child : this->children) {
						child.second->get(&subIter);
					}

					dbus_message_iter_close_container(iter, &subIter);
				}
				*/
			}
			return;

		case DBUS_TYPE_DICT_ENTRY:
			{
				DBusMessageIter subIter;
				if(dbus_message_iter_open_container(iter, DBUS_TYPE_STRUCT, NULL, &subIter)) {

					for(auto child : this->children) {
						child.second->get(&subIter);
					}

					dbus_message_iter_close_container(iter, &subIter);
				}

			}
			return;
		}

		if(!children.empty()) {
			return;
		}

		dbus_message_iter_append_basic(iter,this->type,&this->value);

	}


 }


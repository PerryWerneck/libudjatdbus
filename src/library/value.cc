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
 #include <udjat/tools/dbus.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/logger.h>
 #include <cstring>
 #include <string>
 #include <iostream>

 using namespace std;

 /// @brief Launch runtime error if the dbus type is invalid or unsupported.
 #define EXCEPTION_ON_UNSUPPORTED_OR_INVALID \
			case DBUS_TYPE_INVALID: \
				throw runtime_error("Value is undefined"); \
			case DBUS_TYPE_ARRAY: \
				throw runtime_error("Unsupported DBUS_TYPE_ARRAY value"); \
			case DBUS_TYPE_DICT_ENTRY: \
				throw runtime_error("Unsupported DBUS_TYPE_DICT_ENTRY value"); \
			case DBUS_TYPE_VARIANT: \
				throw runtime_error("Unsupported DBUS_TYPE_VARIANT value");

 namespace Udjat {

 	DBus::Value::Value(const Value *src) {

 		type = src->type;

 		if(type == DBUS_TYPE_STRING) {
			memset(&value,0,sizeof(value));
			value.str = strdup(src->value.str);
 		} else {
			value = src->value;
 		}

	}

	DBus::Value::Value(const Value &src) {

 		type = src.type;

 		if(type == DBUS_TYPE_STRING) {
			memset(&value,0,sizeof(value));
			value.str = strdup(src.value.str);
 		} else {
			value = src.value;
 		}

	}

	DBus::Value::Value() {
		type = DBUS_TYPE_INVALID;
		memset(&value,0,sizeof(value));
	}

	DBus::Value::Value(Message &message) : Value() {
		set(message.getIter());
		message.next();
	}

	DBus::Value::Value(int type, const char *str) : DBus::Value::Value() {

		reset();
		this->type = type;

		if(str) {

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
				value.str = strdup(str);
#ifdef DEBUG
				cout << "value(" << ((char) this->type) << ")='" << value.str << "' (" << ((void *) value.str) << endl;
#endif // DEBUG
				break;

			default:
				throw runtime_error("Unexpected type id");
			}

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

	Udjat::Value & DBus::Value::set(const Udjat::Value UDJAT_UNUSED(&value)) {
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

	bool DBus::Value::set(DBusMessageIter *iter) {

		reset(Value::Type::Undefined);

		type = dbus_message_iter_get_arg_type(iter);

		switch(type) {
		case DBUS_TYPE_INVALID:
			return false;

		case DBUS_TYPE_ARRAY:
			cerr << "d-bus\tUnsupported DBUS_TYPE_ARRAY value" << endl;
			reset(Value::Type::Undefined);
			return false;

		case DBUS_TYPE_DICT_ENTRY:
			cerr << "d-bus\tUnsupported DBUS_TYPE_DICT_ENTRY value" << endl;
			reset(Value::Type::Undefined);
			return false;

		case DBUS_TYPE_VARIANT:
			cerr << "d-bus\tUnsupported DBUS_TYPE_VARIANT value" << endl;
			reset(Value::Type::Undefined);
			return false;

		default:
			dbus_message_iter_get_basic(iter,&value);
			if(type == DBUS_TYPE_STRING) {
				// String, copy the value.
				char * dup = strdup(value.str);
				value.str = dup;
			}

		}

		return true;

	}

	void DBus::Value::get(DBusMessageIter *iter) const {

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

#ifdef DEBUG
		if(this->type == DBUS_TYPE_STRING) {
			cout << "Value(" << ((char) this->type) << ") = '" << this->value.str << "' (" << ((void *) this->value.str) << ")" << endl;
		}
#endif // DEBUG

		if(!dbus_message_iter_append_basic(iter,this->type,&this->value)) {
			throw runtime_error("Can't add value to d-bus iterator");
		}

	}

	const Udjat::Value & DBus::Value::get(std::string &value) const {

		switch(this->type) {
		EXCEPTION_ON_UNSUPPORTED_OR_INVALID

		case DBUS_TYPE_STRING:
			value = this->value.str;
			break;

		case DBUS_TYPE_BOOLEAN:
			value = (this->value.bool_val ? "true" : "false");
			break;

		default:
			throw runtime_error("Unable to convert dbus value to string");
		}

		return *this;

	}

	const Udjat::Value & DBus::Value::get(bool &value) const {

		if(this->type == DBUS_TYPE_BOOLEAN) {
			value = this->value.bool_val;

		} else if(this->type == DBUS_TYPE_STRING) {

			char str = toupper(this->value.str[0]);
			if(str == 'T' || str == 'V' || str == '1') {
				value = true;
			} else if(str == 'F' || str == '0') {
				value = false;
			} else {
				throw runtime_error(string{"Cant convert '"} + this->value.str + "' to boolean");
			}

		} else {

			unsigned int v;
			get(v);
			value = (v != 0);

		}

		return *this;
	}

	int DBus::Value::getFD() const {

		if(type != DBUS_TYPE_UNIX_FD) {
			throw runtime_error("Value is not a file handle");
		}

		return value.fd;
	}

	const Udjat::Value & DBus::Value::get(unsigned int &value) const {

		switch(this->type) {
		EXCEPTION_ON_UNSUPPORTED_OR_INVALID

		case DBUS_TYPE_STRING:
			value = (unsigned int) stoi(this->value.str);
			break;

		case DBUS_TYPE_BOOLEAN:
			value = (this->value.bool_val ? 1 : 0);
			break;

		case DBUS_TYPE_INT16:
			value = (unsigned int) this->value.i16;
			break;

		case DBUS_TYPE_INT32:
			value = (unsigned int) this->value.i32;
			break;

		case DBUS_TYPE_INT64:
			value = (unsigned int) this->value.i64;
			break;

		case DBUS_TYPE_UINT16:
			value = this->value.u16;
			break;

		case DBUS_TYPE_UINT32:
			value = this->value.u32;
			break;

		case DBUS_TYPE_UINT64:
			value = this->value.u64;
			break;

		case DBUS_TYPE_DOUBLE:
			value = (unsigned int) this->value.dbl;
			break;

		default:
			throw runtime_error( string{"Unable to convert dbus value "} + ((char) type) + " to unsigned int");

		}

		return *this;

	}

 }


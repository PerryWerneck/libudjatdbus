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

 /**
  * @brief Implement D-Bus service object.
  */

 // References:
 //
 // https://github.com/fbuihuu/samples-dbus/blob/master/dbus-server.c
 //

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/version.h>
 #include <dbus/dbus.h>
 #include <stdexcept>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/exception.h>
 #include <udjat/tools/value.h>

 #include <udjat/tools/service.h>
 #include <udjat/tools/worker.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/exception.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/interface.h>
 #include <udjat/tools/timestamp.h>

 #include <udjat/tools/dbus/defs.h>
 #include <udjat/tools/dbus/connection.h>
 #include <udjat/tools/dbus/message.h>
 #include <udjat/tools/dbus/service.h>
 #include <udjat/tools/dbus/exception.h>

 #include <sstream>

 using namespace std;

 namespace Udjat {

	DBus::Service::Interface & DBus::Service::interface(const char *intfname) {
		for(Interface &interface : interfaces) {
			debug("Checking '",interface.interface(),"' -> '",intfname,"'");
			if(!strcasecmp(interface.interface(),intfname)) {
				return interface;
			} 
		}
		throw system_error(ENOENT,system_category(),String{"Cant find interface '",intfname,"'"});
	}

	bool DBus::Service::Interface::introspect(Udjat::String &xmldata) const {

		xmldata += "<interface name=\"";
		xmldata += interface();
		xmldata += "\">";

		for(const auto &handler : *this) {
			xmldata += "<method name=\"";
			xmldata += handler.name();
			xmldata += "\">";

			handler.for_each([&](const Interface::Handler::Introspection &introspection){

				if(introspection.direction & Interface::Handler::Introspection::FromPath) {
					return false;
				}

				static const char *text[] = {
					"in",
					"out"
				};

				static const Interface::Handler::Introspection::Direction direction[] = {
					Interface::Handler::Introspection::Input, 
					Interface::Handler::Introspection::Output
				};

				for(size_t ix = 0; ix < 2;ix++) {
					if(introspection.direction & direction[ix]) {
						xmldata += "<arg name=\"";
						xmldata += introspection.name;
						xmldata += "\" type=\"";
						
						xmldata += "s"; /// FIXME: Use the correct data type from instrospection.type.

						xmldata += "\" direction=\"";
						xmldata += text[ix];
						xmldata += "\"/>";
					}
				}

				return false;

			});

			xmldata += "</method>";
		}

		xmldata += "</interface>";

		return true;
	}

	DBus::Service::Interface::Interface(const XML::Node &node, const char *in) 
		: Udjat::Interface{node}, intfname{in} {
		Logger::String("Registering interface ",intfname).trace();
	}

	DBus::Service::Interface::~Interface() {
	}

	bool DBus::Service::Interface::push_back(const XML::Node &node, std::shared_ptr<Action> action) {
		push_back(node).push_back(action);
		return true;
	}

	Udjat::Interface::Handler & DBus::Service::Interface::push_back(const XML::Node &node) {
		return emplace_back(node);
	}

	/// @brief Import value from iter to Udjat::Value
	/// @param iter The iter pointing to input value.
	/// @param value The request element.
	static void import_value(int type, DBusMessageIter *iter, Udjat::Value &value) {

		DBusBasicValue dbval;

		switch(type) {
		case DBUS_TYPE_INVALID:
			throw runtime_error("Required argument not found");

		case DBUS_TYPE_STRING:
			dbus_message_iter_get_basic(iter,&dbval);
			value.set(dbval.str,(Value::Type) value);
			break;

		case DBUS_TYPE_BOOLEAN:
			dbus_message_iter_get_basic(iter,&dbval);
			value.set(dbval.bool_val != 0);
			break;

		case DBUS_TYPE_INT16:
			dbus_message_iter_get_basic(iter,&dbval);
			value.set((int) dbval.i16);
			break;

		case DBUS_TYPE_INT32:
			dbus_message_iter_get_basic(iter,&dbval);
			value.set((int) dbval.i32);
			break;

		case DBUS_TYPE_INT64:
			dbus_message_iter_get_basic(iter,&dbval);
			value.set((int) dbval.i64);
			break;

		case DBUS_TYPE_UINT16:
			dbus_message_iter_get_basic(iter,&dbval);
			value.set((unsigned int) dbval.u16);
			break;

		case DBUS_TYPE_UINT32:
			dbus_message_iter_get_basic(iter,&dbval);
			value.set((unsigned int) dbval.u32);
			break;

		default:
			throw runtime_error("Unexpected argument type");

		}

	}

	static void export_value(int type, DBusMessageIter *iter, Udjat::Value &value) {

		DBusBasicValue dbval;

		switch(type) {
		case Value::String:
		case Value::Icon:
		case Value::Url:
			{
				string str = value.to_string();
				dbval.str = (char *) str.c_str();
				dbus_message_iter_append_basic(iter,DBUS_TYPE_STRING,&dbval.str);
			}
			break;

		case Value::Timestamp:
			{
				time_t tm;
				value.get(tm);
				string str = TimeStamp{tm}.to_string(TIMESTAMP_FORMAT_JSON);
				dbval.str = (char *) str.c_str();
				dbus_message_iter_append_basic(iter,DBUS_TYPE_STRING,&dbval.str);
			}
			break;

		case Value::Signed:
			{
				int val;
				value.get(val);
				dbval.i32 = (int32_t) val;
				dbus_message_iter_append_basic(iter,DBUS_TYPE_INT32,&dbval.i32);
			}
			break;

		case Value::Unsigned:
			{
				unsigned int val;
				value.get(val);
				dbval.u32 = (uint32_t) val;
				dbus_message_iter_append_basic(iter,DBUS_TYPE_UINT32,&dbval.u32);
			}
			break;

		case Value::Boolean:
			{
				unsigned int val;
				value.get(val);
				dbval.bool_val = (val != 0);
				dbus_message_iter_append_basic(iter,DBUS_TYPE_BOOLEAN,&dbval.bool_val);
			}
			break;

		default:
			throw runtime_error("Unsupported output value type");
		}

	}

	static DBusHandlerResult call(DBusConnection *connct, DBusMessage *message, Udjat::Interface::Handler &handler) noexcept {

		DBusMessage *response = NULL;

		try {

			debug("Running method '",handler.name(),"'");

			// Build request.
			Udjat::Request request{dbus_message_get_path(message)};
			{
				DBusMessageIter iter;
				if(dbus_message_iter_init(message,&iter)) {
					handler.for_each([&](const Interface::Handler::Introspection &instrospection){

						if(instrospection.direction & Interface::Handler::Introspection::FromPath) {
							return false;
						}

						if(instrospection.direction & Interface::Handler::Introspection::Input) {
							auto type = dbus_message_iter_get_arg_type(&iter);
							if(type == DBUS_TYPE_INVALID) {
								throw runtime_error(Logger::String{"Required argument '",instrospection.name,"' is missing"});
							}
							auto &value = request[instrospection.name];
							value.clear(instrospection.type);
							import_value(type,&iter,value);
							dbus_message_iter_next(&iter);
						}

						return false;
					});
				} else {
					handler.for_each([&](const Interface::Handler::Introspection &introspection){
						if(introspection.direction & Interface::Handler::Introspection::Input) {
							if(introspection.direction & Interface::Handler::Introspection::FromPath) {
								return true;
							}
							throw runtime_error(Logger::String{"Required argument '",introspection.name,"' is missing"});
						}
						return false;
					});
				}
			}

			// Build response.
			Udjat::Response rsp;


			// Call handler.
			int rc = handler.call(request,rsp);
			if(rc) {
				throw runtime_error(Logger::String{"Action '",handler.name(),"' failed with rc=",rc});
			}

			// Build response
			response = dbus_message_new_method_return(message);
			try {
				DBusMessageIter iter;
 				dbus_message_iter_init_append(response, &iter);
				handler.for_each([&](const Interface::Handler::Introspection &introspection){
					if(introspection.direction & Interface::Handler::Introspection::Output) {
						export_value(introspection.type,&iter,rsp[introspection.name]);
					}
					return false;
				});
			} catch(...) {
				dbus_message_unref(response);
				response = NULL;
				throw;
			}

		} catch(const std::exception &e) {
			response = 
				dbus_message_new_error(
					message,
					DBUS_ERROR_FAILED,
					e.what()
				);
		} catch(...) {
			response = 
				dbus_message_new_error(
					message,
					DBUS_ERROR_FAILED,
					"Unexpected error running handler"
				);
		}

		dbus_connection_send(connct, response, NULL);
		dbus_message_unref(response);

		return DBUS_HANDLER_RESULT_HANDLED;

	}

	DBusHandlerResult DBus::Service::Interface::on_message(DBusConnection *connct, DBusMessage *message, DBus::Service &service) {

		const char *name = dbus_message_get_member(message);
		for(Udjat::Interface::Handler &handler : *this) {
			if(handler == name) {
				return call(connct,message,handler);
			}
		}


		//
		// Not found, return error.
		//
		Logger::String{"Cant handle ",dbus_message_get_interface(message),".",name}.warning(this->name());
		DBusMessage *response = 
			dbus_message_new_error(
				message,
				DBUS_ERROR_FAILED,
				String{"Cant find member '",dbus_message_get_member(message),"'"}.c_str()
			);

		dbus_connection_send(connct, response, NULL);
		dbus_message_unref(response);

		return DBUS_HANDLER_RESULT_HANDLED;
	}

 }

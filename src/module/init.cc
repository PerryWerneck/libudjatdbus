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
 #include <udjat/module/abstract.h>
 #include <udjat/moduleinfo.h>
 #include <udjat/factory.h>
 #include <dbus/dbus-protocol.h>
 #include <udjat/alert/d-bus.h>
 #include <memory>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/dbus/connection.h>
 #include <udjat/tools/worker.h>
 #include <udjat/tools/request.h>
 #include <udjat/tools/response/object.h>

 using namespace std;
 using namespace Udjat;

 static const Udjat::ModuleInfo moduleinfo { "D-Bus" STRINGIZE_VALUE_OF(DBUS_MAJOR_PROTOCOL_VERSION) " module" };

 class Module : public Udjat::Module, Udjat::Factory {
 private:

#ifdef DEBUG
	DBusBusType bustype = DBUS_BUS_STARTER;
	const char *service_name = "br.eti.werneck.udjat";
#else
	DBusBusType bustype = (DBusBusType) -1;
	const char *service_name = nullptr;
#endif // DEBUG

 public:

	Module() : Udjat::Module("d-bus",moduleinfo), Udjat::Factory("d-bus",moduleinfo) {
	};

	Module(const XML::Node &node) : Module() {

		static DBusBusType types[] = {
			DBUS_BUS_SESSION,	///< @brief The login session bus.
			DBUS_BUS_SYSTEM,	///< @brief The systemwide bus.
			DBUS_BUS_STARTER	///< @brief The bus that started us, if any.
		};

		size_t type = String(node,"bus-name",(node.attribute("service").as_bool() ? "starter" : "none")).select("session","system","starter","none",NULL);

		if(type >= (sizeof(types)/sizeof(types[0]))) {
			Logger::String{"Attribute 'bus-name' is empty or invalid, no listener will be activated"}.trace("d-bus");
			return;
		}

		//
		// Get service name
		//
		String srvname{node,"dbus-service-name",""};

		if(srvname.empty()) {
			srvname = ".";
			srvname += Application::Name{};
		}

		if(srvname[0] == '.') {
			Config::Value<string> prefix{"dbus","service-prefix","br.eti.werneck"};
			srvname = prefix + srvname.c_str();
		}

		this->service_name = srvname.as_quark();

		Logger::String{"Service name set to ",this->service_name}.info("d-bus");

	}

	virtual ~Module() {
	};

	std::shared_ptr<Abstract::Alert> AlertFactory(const Abstract::Object &parent, const pugi::xml_node &node) const override {
		return make_shared<Udjat::DBus::Alert>(parent,node);
	}


	void set(std::shared_ptr<Abstract::Agent> agent) noexcept override {

		if(bustype ==(DBusBusType) -1 || !(service_name && *service_name)) {
			return;
		}

		debug("---------------------------------------------------------> Creating service");

		// D-Bus service.
		class Service : private Abstract::DBus::Connection, public Udjat::NamedObject {
		private:
			const char *service_name;

			DBusHandlerResult filter(DBusMessage *message) override {

				if(dbus_message_get_type(message) != DBUS_MESSAGE_TYPE_METHOD_CALL) {
					return Abstract::DBus::Connection::filter(message);
				}

				const char * interface = dbus_message_get_interface(message);
				debug("----> Interface=",interface);

				{
					size_t sz = strlen(service_name);

					if(strlen(interface) <= sz || interface[sz] != '.' || strncasecmp(interface,service_name,sz)) {
						Logger::String("Rejecting interface '",interface,"'");
						return Abstract::DBus::Connection::filter(message);
					}

					interface += (sz+1);
					debug("Interface name: '",interface,"'");

				}

				DBusMessage *reply = nullptr;	///< @brief The message reply.

				try {

					const Worker &worker{Udjat::Worker::find(interface)};

					debug("Got worker '",worker.c_str(),"'");
					debug("Path='",dbus_message_get_path(message),"'");
					debug("Member='",dbus_message_get_member(message),"'");

					const char *member = dbus_message_get_member(message);
					if(strcasecmp(member,"get")) {

						// Only 'get' member is valid.
						reply = dbus_message_new_error(message,DBUS_ERROR_UNKNOWN_METHOD,Logger::String{"Invalid member '",member,"'"}.c_str());

					} else {

						Request request{dbus_message_get_path(message)};
						Response::Object response;

						worker.get(request,response);

						// Set response message.
						reply = dbus_message_new_method_return(message);
						try {


						} catch(...) {

							dbus_message_unref(reply);
							reply = NULL;

						}

					}


				} catch(const std::system_error &e) {

					Logger::String{"Cant handle '",dbus_message_get_interface(message),"': ",e.what()," (rc=",e.code().value(),")"}.error("d-bus");
					reply = dbus_message_new_error(message,DBUS_ERROR_FAILED,e.what());

				} catch(const std::exception &e) {

					Logger::String{"Cant handle '",dbus_message_get_interface(message),"': ",e.what()}.error("d-bus");
					reply = dbus_message_new_error(message,DBUS_ERROR_FAILED,e.what());

				} catch(...) {

					Logger::String{"Cant handle '",dbus_message_get_interface(message),"': Unexpected error"}.error("d-bus");
					reply = dbus_message_new_error(message,DBUS_ERROR_FAILED,"Unexpected error");

				}

				if(reply) {
					dbus_uint32_t serial = 0;
					if(!dbus_connection_send(conn, reply, &serial)) {
						Logger::String{"Cant send response to '",dbus_message_get_interface(message),"'"}.error("d-bus");
						return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
					}
					dbus_connection_flush(conn);
					dbus_message_unref(reply);
					return DBUS_HANDLER_RESULT_HANDLED;
				}

				return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

			}

		public:
			Service(DBusBusType bustype, const char *sname) : Abstract::DBus::Connection{"d-bus",SharedConnectionFactory(bustype)}, Udjat::NamedObject{"d-bus"}, service_name{sname} {
				Logger::String{"Starting ",service_name}.info("d-bus");
				if(request_name(service_name) != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
					throw runtime_error(Logger::String{"Cant get ownership of ",service_name});
				}
				open();

			}

			virtual ~Service() {
				Logger::String{"Stopping ",service_name}.info("d-bus");
				close();
				dbus_connection_unref(conn);
			}

		};

		// New root agent, create d-bus service and add to it.
		try {

			agent->push_back(make_shared<Service>(bustype,service_name));

		} catch(const std::exception &e) {

			Logger::String{"Error '",e.what(),"' starting d-bus service"}.error("d-bus");

		} catch(...) {


			Logger::String{"Unexpected error starting d-bus service"}.error("d-bus");

		}


	}

 };

 /// @brief Register udjat module.
 Udjat::Module * udjat_module_init() {
	return new ::Module();
 }

 Udjat::Module * udjat_module_init_from_xml(const pugi::xml_node &node) {
	return new ::Module(node);
 }


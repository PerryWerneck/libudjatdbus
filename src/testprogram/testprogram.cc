/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tests.h>
 #include <udjat/moduleinfo.h>
 #include <udjat/module.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/dbus.h>
 #include <udjat/tools/dbus/connection.h>
 #include <udjat/tools/dbus/message.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/dbus/service.h>
 #include <udjat/tools/dbus/signal.h>
 #include <udjat/module/dbus.h>
 #include <udjat/tools/logger.h>

 using namespace std;
 using namespace Udjat;
 using namespace Udjat::DBus;

 int main(int argc, char **argv) {

	static const ModuleInfo info{"dbus-tester"};
	
	return Testing::run(argc,argv,info,[](Application &){

		class Module : public DBus::Module, public DBus::Service {
		public:
			Module()
				: DBus::Module{},
					DBus::Service{
						(const ModuleInfo &) *this,
						DBus::Connection::getInstance(DBUS_BUS_STARTER),
						"dbus",
						String{PRODUCT_ID,".",Application::Name().c_str()}.as_quark()
					} { }

			virtual ~Module() {
			}

		};

		new Module();

		// 2 asynchronous calls chained.
		SystemBus::getInstance().call(
			DBus::Message{
				"org.freedesktop.login1",
				"/org/freedesktop/login1",
				"org.freedesktop.login1.Manager",
				"GetSession",
				"2"
			},
			[](DBus::Message & message) -> void {

				message.except();
				std::string session_path;
				message.pop(session_path);
				debug("Session-path=",session_path.c_str());

				SystemBus::getInstance().call(
					DBus::Message{
						"org.freedesktop.login1",
						session_path.c_str(),
						"org.freedesktop.DBus.Properties",
						"Get",
						"org.freedesktop.login1.Session",
						"LockedHint"
					},
					[](DBus::Message & message) -> void {

						message.except();
						bool locked;
						message.pop(locked);
						Logger::String{"Session is ",(locked ? "locked" : "unlocked")}.info();

					}
				);

			}
		);

		/*
		SystemBus::getInstance().get(
			"org.freedesktop.systemd1",
			"/org/freedesktop/systemd1",
			"org.freedesktop.systemd1.Manager",
			"Virtualization",
			[](Udjat::DBus::Message & message) {

				if(message) {

					string response;
					message.pop(response);

					debug("-------------------------> Got response Virtualization=",response);

				} else {

					debug("-------------------------> Error calling org.freedesktop.systemd1");

				}

			}
		);

		SessionBus::getInstance().call(
			"org.gnome.ScreenSaver",
			"/org/gnome/ScreenSaver",
			"org.gnome.ScreenSaver",
			"GetActiveTime",
			[](DBus::Message & message) {

				if(message) {

					unsigned int active;
					message.pop(active);

					debug("-------------------------> Got response active=",active);

				} else {

					debug("-------------------------> Error calling gnome");

				}

			});
		*/

		SessionBus::getInstance().subscribe(
				"org.gnome.ScreenSaver",
				"ActiveChanged",
				[](DBus::Message &message) {
					bool active;
					message.pop(active);
					Logger::String{"Gnome screensaver is now ",(active ? "active" : "inactive")}.info("d-bus");
					return false;
				}
		);

		SessionBus::getInstance().subscribe(
				"org.gnome.ScreenSaver",
				"WakeUpScreen",
				[](DBus::Message &) {
					Logger::String{"Gnome screen saver WakeUpScreen signal"}.trace();
					return false;
				}
		);

		SessionBus::getInstance().subscribe("com.example.signal","hello",[](DBus::Message &message){
			cout << "Got signal hello with message '" << message << "'" << endl;
			return false;
		});

		// Check user bus
		debug("------------------------------------------------------------");
		UserBus{1000}.signal(
			DBus::Signal{
				"com.example.signal.user",
				"hello",
				"/userbus",
				"userbus-message"
			}
		);
		debug("------------------------------------------------------------");

	});

 }


/*
 #include <config.h>
 #include <udjat/tools/application.h>
 #include <udjat/module.h>
 #include <unistd.h>
 #include <udjat/version.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/dbus/connection.h>
 #include <udjat/tools/dbus/message.h>
 #include <udjat/tools/dbus/service.h>
 #include <udjat/tools/threadpool.h>

 #if UDJAT_CHECK_VERSION(1,2,0)
	#include <udjat/tools/factory.h>
 #else
	#include <udjat/factory.h>
 #endif // UDJAT_CHECK_VERSION

 using namespace std;
 using namespace Udjat;
 using namespace Udjat::DBus;

 static const Udjat::ModuleInfo moduleinfo { "Test program" };

 class RandomFactory : public Udjat::Factory {
 public:
	RandomFactory() : Udjat::Factory("random",moduleinfo) {
		cout << "random agent factory was created" << endl;
		srand(time(NULL));
	}

	std::shared_ptr<Abstract::Agent> AgentFactory(const Abstract::Object UDJAT_UNUSED(&parent), const XML::Node &node) const override {

		class RandomAgent : public Agent<unsigned int> {
		private:
			unsigned int limit = 5;

		public:
			RandomAgent(const XML::Node &node) : Agent<unsigned int>(node) {
			}

			bool refresh() override {
				debug("Updating agent '",name(),"'");
				set( ((unsigned int) rand()) % limit );
				return true;
			}

			void start() override {
				Agent<unsigned int>::start( ((unsigned int) rand()) % limit );
			}

		};

		return make_shared<RandomAgent>(node);

	}

 };

 int main(int argc, char **argv) {

	Logger::verbosity(9);
	Logger::redirect();
	Logger::console(true);

	udjat_module_init();

	SystemBus bus;
	bus.get(
		"org.freedesktop.systemd1",
		"/org/freedesktop/systemd1",
		"org.freedesktop.systemd1.Manager",
		"Virtualization",
		[](Udjat::DBus::Message & message) {

			if(message) {

				string response;
				message.pop(response);

				debug("-------------------------> Got response Virtualization=",response);

			} else {

				debug("-------------------------> Error calling org.freedesktop.systemd1");

			}

		}
	);

	UserBus bus{1000};
	//SessionBus bus;

	bus.call(
		"org.gnome.ScreenSaver",
		"/org/gnome/ScreenSaver",
		"org.gnome.ScreenSaver",
		"GetActiveTime",
		[](DBus::Message & message) {

			if(message) {

				unsigned int active;
				message.pop(active);

				debug("-------------------------> Got response active=",active);

			} else {

				debug("-------------------------> Error calling gnome");

			}

		});

	bus.subscribe(
			"org.gnome.ScreenSaver",
			"ActiveChanged",
			[](DBus::Message &message) {

				// Active state of gnome screensaver has changed, deal with it.
				bool locked = DBus::Value(message).as_bool();
				Logger::String{"Gnome screensaver is now ",(locked ? "active" : "inactive")}.info("d-bus");

				return false;

			}
	);

	udjat_module_init();
	RandomFactory rfactory;


	{
		NamedBus nbus1{"named1",getenv("DBUS_SESSION_BUS_ADDRESS")};
		nbus1.subscribe(
				"org.gnome.ScreenSaver",
				"ActiveChanged",
				[](DBus::Message &message) {

					// Active state of gnome screensaver has changed, deal with it.
					bool locked = DBus::Value(message).as_bool();
					Logger::String{"Gnome screensaver is now ",(locked ? "active" : "inactive")}.info("d-bus");

					return false;

				}
		);
	}

	try {
		// https://www.freedesktop.org/wiki/Software/systemd/logind/
		debug("------------------------------ Check if can power off -------------------------");
		SystemBus::getInstance().call_and_wait(
			DBus::Message{
				"org.freedesktop.login1",
				"/org/freedesktop/login1",
				"org.freedesktop.login1.Manager",
				"CanPowerOff"
			},
			[](Udjat::DBus::Message & response) {

				if(response) {
					std::string value;
					response.pop(value);
					debug("Got response: ",value);
				} else {
					cerr << "Error" << endl;
				}

			}
		);
		debug("----------------------------xxxxxxxxxxxxxxxxxx--------------------");


	} catch(const std::exception &e) {

		cerr << "---> " << e.what() << endl;
	}

	{
		SystemBus sbus0;
		SystemBus sbus1;
		SystemBus sbus2;
		//SystemBus sbus3;
		//SystemBus sbus4;
		//SystemBus sbus5;
		//SystemBus sbus6;
	}

	SystemBus bus;
	bus.subscribe(
			"org.freedesktop.login1.Manager",
			"SessionNew",
			[](DBus::Message &message) {

				debug("----------------------------------> SessionNew");

				string sid;
				message.pop(sid);

				string path;
				message.pop(path);

				cout << "users\t Session '" << sid << "' started on path '" << path << "'" << endl;

				return false;

			}
	);

	Udjat::DBus::SessionBus::getInstance().subscribe("com.example.signal","hello",[](DBus::Message &){

		cout << "Got signal hello" << endl;
		return false;

	});


	DBus::Service srvc{moduleinfo,"service","br.eti.werneck.udjat"};

	auto rc = Application{}.run(argc,argv,"./test.xml");
	debug("Application exits with rc=",rc);

	return rc;
}

*/

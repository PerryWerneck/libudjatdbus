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

 #include <udjat/tools/systemservice.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/dbus.h>
 #include <udjat/agent.h>
 #include <udjat/factory.h>
 #include <udjat/module.h>
 #include <iostream>
 #include <memory>
 #include <cstdlib>
 #include <unistd.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/logger.h>

 using namespace std;
 using namespace Udjat;

//---[ Implement ]------------------------------------------------------------------------------------------

int run_as_service(int argc, char **argv) {

	static const Udjat::ModuleInfo moduleinfo { "Test program" };

	class RandomFactory : public Udjat::Factory {
	public:
		RandomFactory() : Udjat::Factory("random",moduleinfo) {
			cout << "random agent factory was created" << endl;
			srand(time(NULL));
		}

		std::shared_ptr<Abstract::Agent> AgentFactory(const Abstract::Object UDJAT_UNUSED(&parent), const pugi::xml_node &node) const override {

			class RandomAgent : public Agent<unsigned int> {
			private:
				unsigned int limit = 5;

			public:
				RandomAgent(const pugi::xml_node &node) : Agent<unsigned int>(node) {
				}

				bool refresh() override {

					unsigned int value = ((unsigned int) rand()) % limit;
					debug("Updating agent '",name(),"' to ",value);

					return set(value);

				}

				void start() override {
					Agent<unsigned int>::start( ((unsigned int) rand()) % limit );
				}

			};

			return make_shared<RandomAgent>(node);

		}

	};

	class Service : public SystemService {
	private:
		DBus::Connection *bus = nullptr;
		RandomFactory rfactory;

	protected:
		/// @brief Initialize service.
		void init() override {
			cout << Application::Name() << "\tInitializing" << endl;

			udjat_module_init();

			SystemService::init();

			/*
			{
				DBus::Message message{
					"org.freedesktop.login1",			// Destination
					"/org/freedesktop/login1",			// Path
					"org.freedesktop.login1.Manager",	// Interface
					"Inhibit"							// Method
				};

				message	<< "sleep"
						<< "who"
						<< "why"
						<< "delay";

				// Get system bus
				cerr << "------------------- calling system bus" << endl;
				DBus::Connection::getSystemInstance().call(message,[](DBus::Message &response){

					if(response) {

						debug("SUCCESS");
						int fd = DBus::Value(response).getFD();

						debug("FD=",fd);

						::close(fd);

					} else {

						debug("FAILED");

					}

				});

			}
			*/

			DBus::Connection &bus = DBus::Connection::getSessionInstance();

			/*
			cout << "----------------------------- Invalid message" << endl;
			bus.call(
				"br.eti.werneck.invalid",
				"/service/none",
				"br.eti.werneck.invalid",
				"invalid",
				[](DBus::Message & message) {
					debug(message.error_message());
				}
			);
			*/

			/*
			cout << "----------------------------- org.gnome.ScreenSaver" << endl;
			bus.call(
				"org.gnome.ScreenSaver",
				"/org/gnome/ScreenSaver",
				"org.gnome.ScreenSaver",
				"GetActiveTime",
				[](DBus::Message & message) {

					if(message) {

						unsigned int active;
						message.pop(active);

						cout << "org.gnome.ScreenSaver.ActiveTime=" << active << endl;

					} else {

						cerr << "Error '" << message.error_message() << "' getting screensaver active time" << endl;
					}
				}
			);
			*/

			bus.subscribe(
				this,
				"org.gnome.ScreenSaver",
				"ActiveChanged",
				[](DBus::Message &message) {

					cout << "org.gnome.ScreenSaver.ActiveChanged " << DBus::Value(message).to_string() << endl;

				}
			);

			bus.subscribe(
				this,
				"com.example.signal",
				"hello",
				[](DBus::Message &message) {

					cout << "com.example.signal.hello " << DBus::Value(message).to_string() << endl;

				}
			);

			/*
			DBus::Signal(
				"com.example.signal.welcome",
				"test",
				"/agent/simple"
			)
				.push_back("Simple D-Bus signal")
				.push_back((uint16_t) 10)
				.send();
			*/

			DBus::Signal(
				"com.example.signal.welcome",
				"test",
				"/agent/simple",
				"Simple D-Bus signal",
				(uint16_t) 10
			).send();

			// Test user bus
			{
				cout << "------------------------------------------------" << endl;
				DBus::Connection usercon((uid_t) 1000);
			}

			// Test notification
			cout << "---[ Message Test Begin ]--------------------------------------------" << endl;
			try {

				DBus::Message message{
					"org.freedesktop.Notifications",		// Destination
					"/org/freedesktop/Notifications",		// Path
					"org.freedesktop.Notifications",		// Interface
					"Notify",								// Method
					PACKAGE_NAME,
					((unsigned int) 0),
					"gtk-dialog-info",
					"Remote instalation service",
					"This machine is acting as an installation server, keep it active",
					std::vector<std::string>()
				};

				DBus::Connection::getSessionInstance().call_and_wait(message,[](DBus::Message &response){

					if(response.failed()) {

						error() << "Error '" << response.error_name() << "' sending notification"
								<< endl << response.error_message() << endl;

					} else {

						info() << "Success??" << endl;
					}


				});
			} catch(const std::exception &e) {

				cout << "-----------------------------" << endl << e.what() << endl << "------------------------------" << endl;
			}

			try {

				DBus::Message message{
					"org.freedesktop.Notifications",		// Destination
					"/org/freedesktop/Notifications",		// Path
					"org.freedesktop.Notifications",		// Interface
					"Notify"								// Method
				};

				message	<< PACKAGE_NAME
						<< ((unsigned int) 0)
						<< "gtk-dialog-info"
						<< "Remote instalation service"
						<< "This machine is acting as an installation server, keep it active"
						<< std::vector<std::string>();

				DBus::Connection::getSessionInstance().call_and_wait(message,[](DBus::Message &response){

					if(response.failed()) {

						error() << "Error '" << response.error_name() << "' sending notification"
								<< endl << response.error_message() << endl;

					} else {

						info() << "Success??" << endl;
					}

				});
			} catch(const std::exception &e) {

				cout << "-----------------------------" << endl << e.what() << endl << "------------------------------" << endl;
			}

			cout << "---[ Message Test Finish ]--------------------------------------------" << endl;

		}

		/// @brief Deinitialize service.
		void deinit() override {
			delete bus;
			cout << Application::Name() << "\tDeinitializing" << endl;
			Udjat::Module::unload();
		}

	public:
		Service() : SystemService{"./test.xml"} {
		}

		virtual ~Service() {
		}

	};

	return Service().run(argc,argv);


}

int main(int argc, char **argv) {

	run_as_service(argc,argv);

	//Udjat::DBus::Connection::getSystemInstance();

}

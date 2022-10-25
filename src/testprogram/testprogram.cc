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

	class Service : public SystemService {
	private:
		DBus::Connection *bus = nullptr;

	protected:
		/// @brief Initialize service.
		void init() override {
			cout << Application::Name() << "\tInitializing" << endl;

			SystemService::init();

			if(Module::find("httpd")) {

				cout << "http://localhost:8989/" << endl;

				if(Module::find("information")) {
					cout << "http://localhost:8989/api/1.0/info/modules.xml" << endl;
					cout << "http://localhost:8989/api/1.0/info/workers.xml" << endl;
					cout << "http://localhost:8989/api/1.0/info/factories.xml" << endl;
				}
				cout << "http://localhost:8989/api/1.0/agent.xml" << endl;
				cout << "http://localhost:8989/api/1.0/alerts.xml" << endl;
			}

			/*
			for(auto agent : *root) {
				cout << "http://localhost:8989/api/1.0/agent/" << agent->getName() << ".xml" << endl;
			}
			*/

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

			DBus::Connection &bus = DBus::Connection::getSessionInstance();

			cout << "----------------------------- Invalid message" << endl;
			bus.call(
				"br.eti.werneck.invalid",
				"/service/none",
				"br.eti.werneck.invalid",
				"invalid",
				[](DBus::Message & message) {

					if(message) {

						cout << "*************** Got response, why?!?!" << endl;

					} else {

						cerr << "*************** Error '" << message.error_message() << endl;
					}
				}
			);

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

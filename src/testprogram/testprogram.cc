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
 #include <udjat/tools/application.h>
 #include <udjat/module.h>
 #include <unistd.h>
 #include <udjat/version.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/dbus/connection.h>
 #include <udjat/tools/dbus/message.h>
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

	UserBus bus{1000};
	//SessionBus bus;

	Udjat::DBus::Member *member = &bus.subscribe("com.example.signal","hello",[&member,&bus](DBus::Message &message){

		cout << "Got signal hello" << endl;

		// Cant remove member while running, then, enqueue cleanup.
		ThreadPool::getInstance().push([&member,&bus](){
			bus.remove(*member);
		});

	});

	udjat_module_init();
	RandomFactory rfactory;

	auto rc = Application{}.run(argc,argv,"./test.xml");

	debug("Application exits with rc=",rc);

	return rc;
}

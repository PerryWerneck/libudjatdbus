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

 #include <udjat.h>
 #include <udjat/module.h>
 #include <udjat/factory.h>
 #include <unistd.h>

 using namespace std;
 using namespace Udjat;

 int main(int argc, char **argv) {

	setlocale( LC_ALL, "" );

	Logger::redirect(nullptr,true);

	try {

		Module::load("udjat-module-information");

	} catch(const std::exception &e) {
		cerr << "Error '" << e.what() << "' loading information module" << endl;
	}


	// Randomic value agent factory.
	class Factory : public Udjat::Factory {
	public:
		Factory() : Udjat::Factory("random") {
			srand(time(NULL));
		}

		bool parse(Abstract::Agent &parent, const pugi::xml_node &node) const override {

			class RandomAgent : public Agent<unsigned int> {
			private:
				unsigned int limit = 5;

			public:
				RandomAgent(const pugi::xml_node &node) : Agent<unsigned int>() {
					cout << "Creating random Agent" << endl;
					load(node);
				}

				bool refresh() override {
					cout << "Refreshing random agent" << endl;
					set( (((unsigned int) rand())+1) % limit);
					return true;
				}

			};

			parent.insert(make_shared<RandomAgent>(node));

			return true;

		}

	};

	static Factory factory;

	auto module = udjat_module_init();
	auto agent = Abstract::Agent::init("${PWD}/test.xml");

	Udjat::run();

	Abstract::Agent::deinit();

	cout << "Removing module" << endl;
	delete module;
	Module::unload();

	return 0;
}

/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2025 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tools/testsuite.h>
 #include <udjat/tools/actions/dbus.h>
 #include <udjat/tools/dbus/service.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/service.h>
 #include <udjat/action.h>
 #include <udjat/agent.h>
 #include <memory>

 using namespace Udjat;
 using namespace std;

 #if defined(DEBUG) and ! defined(LIBUDJAT_STATIC) 

 UDJAT_API void udjat_register_tests(Udjat::TestSuite &suite) noexcept {

	using Case = TestSuite::Case;

	suite.add(
		Case{
			"DBus service",
			[](std::ostream &) {

				// Build sample agents
				{
					auto root = Abstract::Agent::RootFactory();
					root->push_back(make_shared<Agent<int>>("intvalue"));
					Abstract::Agent::root(root);
				}

				DBus::Service srvc;

				Service::for_each([](Service &service){
					service.start();
					return false;
				});

				MainLoop::getInstance().run();

				Service::for_each([](Service &service){
					service.stop();
					return false;
				});

				Abstract::Agent::deinit();

				return "Service Ok";
			}
		},
		Case{
			"Emit signal action",
			[](std::ostream &) {

				DBus::Action action{
					DBUS_MESSAGE_TYPE_SIGNAL,
					DBUS_BUS_SESSION,
					"br.eti.werneck.udjat.MyInterface",
					"/br/eti/werneck/udjat/MyObject",
					"br.eti.werneck.udjat.MyInterface",
					"TestSignal"
				};

				Udjat::Request request;
				Udjat::Response response;
				action.call(request,response,true);

				return "Signal emitted";
			}
		}
	);

 }
	
//  static int call_and_wait_test() {

// 	Logger::String{"Emitting signal with action"}.info();
// 	{
// 		DBus::Action action{
// 			DBUS_MESSAGE_TYPE_SIGNAL,
// 			DBUS_BUS_SESSION,
// 			"br.eti.werneck.udjat.MyInterface",
// 			"/br/eti/werneck/udjat/MyObject",
// 			"br.eti.werneck.udjat.MyInterface",
// 			"TestSignal"
// 		};

// 		Udjat::Request request;
// 		Udjat::Response response;
// 		action.call(request,response,true);

// 	}

// 	Logger::String{"Calling gnome screensaver"}.info();
// 	SessionBus::getInstance().call_and_wait(
// 		"org.gnome.ScreenSaver",
// 		"/org/gnome/ScreenSaver",
// 		"org.gnome.ScreenSaver",
// 		"GetActiveTime",
// 		[](DBus::Message & message) {

// 			if(message) {

// 				unsigned int active;
// 				message.pop(active);

// 				debug("-------------------------> Got response active=",active);

// 			} else {

// 				debug("-------------------------> Error calling gnome");

// 			}

// 		});

// 	Logger::String{"Calling gnome screensaver using action"}.info();
// 	{
// 		DBus::Action action{
// 			DBUS_MESSAGE_TYPE_METHOD_CALL,
// 			DBUS_BUS_SESSION,
// 			"org.gnome.ScreenSaver",
// 			"/org/gnome/ScreenSaver",
// 			"org.gnome.ScreenSaver",
// 			"GetActiveTime"
// 		};

// 		Udjat::Request request;
// 		Udjat::Response response;
// 		action.call(request,response,true);

// 	}

// 	Logger::String{"Calling invalid service"}.info();
// 	SessionBus::getInstance().call_and_wait(
// 		"org.invalid.Service",
// 		"/org/invalid/Service",
// 		"org.invalid.Service",
// 		"Method",
// 		[](DBus::Message & message) {

// 			if(message) {

// 				throw runtime_error("Should not get here");

// 			} else {

// 				Logger::String{"No response, as expected: ",message.error_name()," - ",message.error_message()}.info();

// 			}

// 		});


// 	return 0;

//  }

//  UDJAT_API void enum_udjat_unit_tests(Udjat::UnitTests &tests) noexcept {

// 	debug(__FUNCTION__," begin -> ",tests.size());

// 	tests.append(
// 		UnitTests::Worker{
// 			"Call and wait",
// 			[]() {
				
// 				call_and_wait_test();
// 				return true;
// 			}
// 		}
// 	);

//  }

 #endif // DEBUG

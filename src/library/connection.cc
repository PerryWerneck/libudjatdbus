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
 #include <udjat/worker.h>
 #include <iostream>

 using namespace std;

 /*
#include <ipc/dbus.h>
#include <stdexcept>
#include <cstring>
#include <fstream>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <ext/stdio_filebuf.h>
#include <systemd/sd-login.h>

using std::clog;
using std::endl;
using std::cout;
using std::string;
using std::cerr;
*/

 namespace Udjat {

	std::mutex DBus::Connection::guard;

	DBus::Connection::Connection() {
		active = false;
		thread = nullptr;
		connection = nullptr;
	}

	DBus::Connection::Connection(DBusConnection * connection) : Connection() {

		// Register
		DBusError err;
		dbus_error_init(&err);

		dbus_bus_register(connection,&err);
		if(dbus_error_is_set(&err)) {
			std::string message(err.message);
			dbus_error_free(&err);
			throw std::runtime_error(message);
		}

		// Set!
		set(connection);

	}

	DBus::Connection::Connection(DBusBusType type) : Connection() {

		DBusError err;
		dbus_error_init(&err);

		DBusConnection * connct = dbus_bus_get(type, &err);
		if(dbus_error_is_set(&err)) {
			std::string message(err.message);
			dbus_error_free(&err);
			throw std::runtime_error(message);
		}

		set(connct);

	}

	DBus::Connection::Connection(const char *busname) : Connection() {

		DBusError err;
		dbus_error_init(&err);

		DBusConnection * connection = dbus_connection_open(busname, &err);
		if(dbus_error_is_set(&err)) {
			std::string message(err.message);
			dbus_error_free(&err);
			throw std::runtime_error(message);
		}

		try {

			set(connection);

		} catch(...) {
			dbus_connection_unref(connection);
			throw;
		}

	}

	void DBus::Connection::set(DBusConnection * connection) {

		if(this->connection) {
			std::runtime_error("Can't change connection handle");
		}

		this->connection = connection;

		if (dbus_connection_add_filter(connection, (DBusHandleMessageFunction) filter, this, NULL) == FALSE) {
			dbus_connection_unref(connection);
			throw std::runtime_error("Erro ao ativar filtro de mensagens D-Bus");
		}

		// Não encerro o processo ao desconectar.
		dbus_connection_set_exit_on_disconnect(connection, false);
	}

	DBus::Connection::~Connection() {

		active = false;

		// Remove listeners.
		interfaces.remove_if([this](Interface &intf) {

			DBusError error;
			dbus_error_init(&error);

			dbus_bus_remove_match(connection,intf.getMatch().c_str(), &error);
			dbus_connection_flush(connection);

			if (dbus_error_is_set(&error)) {
				std::cerr << "d-bus\t" << error.message << std::endl;
			}

			return true;
		});

		if(connection) {
			// Remove filter
			dbus_connection_remove_filter(connection,(DBusHandleMessageFunction) filter, this);

			// Stop D-Bus connection
			dbus_connection_unref(connection);
		}

		if(thread) {
			// Wait for d-bus thread
			cout << "d-bus\tWaiting for service thread" << endl;
			thread->join();
			delete thread;
			thread = nullptr;
		}

	}

	/// @brief Subscribe to D-Bus signal.
	void DBus::Connection::subscribe(void *id, const char *interface, const char *member, std::function<void(DBusMessage * message)> call) {

		lock_guard<mutex> lock(guard);
		getInterface(interface).members.emplace_back(member,call);

	}

	/*
	/// @brief Obtém valor do ambiente de usuário.
	DBus::Connection * DBus::Connection::get(uid_t uid, const char *sid) {

		DBus::Connection * rc = nullptr;

		if(!uid) {
			return new Connection(DBUS_BUS_SYSTEM);
		}

		// Find user bus.
		// https://stackoverflow.com/questions/6496847/access-another-users-d-bus-session
		{
			DIR * dir = opendir("/proc");
			if(!dir) {
					throw std::system_error(errno, std::system_category());
			}

			struct dirent *ent;
			while((ent=readdir(dir))!=NULL && !rc) {

				if(!atoi(ent->d_name)) {
					continue;
				}

				/// @brief Arquivo no /proc/[PID]/environ
				class Environ {
				private:
					int fd = -1;

				public:
					Environ(DIR * dir, const char *name) : fd(openat(dirfd(dir),(string{name} + "/environ").c_str(),O_RDONLY)) {
	#ifdef DEBUG
						if(fd > 0) {
							cout << "Abri FD " << fd << endl;
						}
	#endif // DEBUG
					}

					~Environ() {

						if(fd > 0) {
	#ifdef DEBUG
							cout << "Fechei FD " << fd << endl;
	#endif // DEBUG
							close(fd);

						}
					}

					operator bool() const {
						return fd >= 0;
					}

					uid_t getUID() {
						struct stat st;
						if(fstat(fd,&st) < 0)
							return (uid_t) -1;
						return st.st_uid;
					}

					int getFD() {
						return this->fd;
					}
				};

				Environ environ(dir,ent->d_name);

				if(!environ || environ.getUID() != uid) {
					continue;
				}

				// Check session ID (if necessary)
				if(sid) {
					char *sname = nullptr;

					// Se o processo não for associado a uma sessão ignora.
					if(sd_pid_get_session(atoi(ent->d_name), &sname) == -ENODATA || !sname)
						continue;

					int sval = strcmp(sid,sname);
					free(sname);

					if(sval)
						continue;

				}

				// Scan for Dbus address
				{
					char buffer[4096];
					ssize_t sz = read(environ.getFD(),buffer,4095);
					if(sz > 0) {
						buffer[sz] = 0;

						static const char *name = "DBUS_SESSION_BUS_ADDRESS";
						static const size_t szName = strlen(name);

						for(const char *ptr = buffer; *ptr; ptr += (strlen(ptr)+1)) {
							if(strncmp(ptr,name,szName) == 0 && ptr[szName] == '=') {

								string busName(ptr+szName+1);

								uid_t saved_uid = geteuid();
								if(seteuid(uid) < 0) {

									cerr << "Can't set efective UID: " << strerror(errno) << endl;

								} else {

									try {

										DBusError err;
										dbus_error_init(&err);

										DBusConnection * connection = dbus_connection_open(busName.c_str(), &err);
										if(dbus_error_is_set(&err)) {
											std::string message(err.message);
											dbus_error_free(&err);
											throw std::runtime_error(message);
										}

										rc = new DBus::Connection(connection);

									} catch(const std::exception &e) {

										cerr << "Can't open D-Bus session " << busName << ": " << e.what() << endl;

									} catch(...) {

										cerr << "Unexpected error opening D-bus session " << busName << endl;

									}

									seteuid(saved_uid);

								}

								break;
							}
						}
					}
				}

			}

			closedir(dir);

		}


		if(!rc) {

			if(sid) {
				throw std::runtime_error(string("Can't find D-Bus address for user ") + std::to_string(uid) + " on session " + sid);
			} else {
				throw std::runtime_error(string("Can't find D-Bus address for user ") + std::to_string(uid));
			}

		}

		return rc;
	}
	*/

	void DBus::Connection::start() {

		lock_guard<mutex> lock(guard);

		if(thread)
			throw std::runtime_error("Connection is already active");

		active = true;

		thread = new std::thread([this]{

#ifdef DEBUG
			cout << "Thread de conexão D-Bus iniciada" << endl;
#endif // DEBUG

			while(active && dbus_connection_read_write(connection,500)) {
				while(connection && dbus_connection_get_dispatch_status(connection) == DBUS_DISPATCH_DATA_REMAINS) {
					dbus_connection_dispatch(connection);
				}
			}

			active = false;

#ifdef DEBUG
			cout << "Thread de conexão D-Bus terminada" << endl;
#endif // DEBUG

		});


	}

	/*
	/// @brief Passagem de parâmetros para método D-Bus.
	struct MethodData {
		std::function<void(DBusMessage * message, DBusError *error)> call;
	};

	static void dbus_call_reply(DBusPendingCall *pending, void *user_data) {

		MethodData *data = (MethodData *) user_data;

		DBusError error;
		dbus_error_init(&error);

		if(!dbus_pending_call_get_completed(pending)) {

			dbus_set_error_const(&error, "Failed", "DBus Error");
			data->call(nullptr,&error);

			dbus_pending_call_cancel(pending);


		} else {

			// Got response
			DBusMessage * message = dbus_pending_call_steal_reply(pending);

			if(message) {
				data->call(message,&error);
				dbus_message_unref(message);
			} else {
				dbus_set_error_const(&error, "empty", "No response");
				data->call(message,&error);
			}

		}

		dbus_error_free(&error);

		delete data;
	}

	void DBus::Connection::call(const char *destination,const char *path, const char *interface, const char *member, std::function<void(DBusMessage * message, DBusError *error)> call) {

		DBusMessage * message = dbus_message_new_method_call(destination,path,interface,member);
		if(message == NULL) {
			throw std::runtime_error("Can't create DBus method call");
		}

		DBusPendingCall *pc = NULL;

		if(!dbus_connection_send_with_reply(connection,message,&pc,DBUS_TIMEOUT_USE_DEFAULT)) {
			throw std::runtime_error("Can't send DBus method call");
		}

		MethodData * data = new MethodData();
		data->call = call;

		if(!dbus_pending_call_set_notify(pc, dbus_call_reply, data, NULL)) {
			dbus_pending_call_unref(pc);
			dbus_message_unref(message);
			delete data;
			throw std::runtime_error("Can't set notify method");
		}

		dbus_pending_call_unref(pc);
		dbus_message_unref(message);

	}
	*/


 }


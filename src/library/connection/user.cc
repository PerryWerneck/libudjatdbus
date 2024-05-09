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
  * @brief Implements system bus connection.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <dbus/dbus.h>
 #include <udjat/tools/dbus/connection.h>
 #include <udjat/tools/logger.h>
 #include <dirent.h>
 #include <string>
 #include <fcntl.h>
 #include <unistd.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <udjat/tools/file.h>
 #include <pwd.h>

 #ifdef HAVE_SYSTEMD
	#include <systemd/sd-login.h>
 #endif // HAVE_SYSTEMD

 using namespace std;

 namespace Udjat {

	static DBusConnection * UserConnectionFactory(uid_t uid, const char *sid) {

		/// @brief File on /proc/[PID]/environ

		Logger::String{"Opening connection to user '",uid,"'"}.trace("d-bus");

		class Environ {
		private:
			int descriptor = -1;

		public:
			Environ(DIR * dir, const char *name) : descriptor(openat(dirfd(dir),(string{name} + "/environ").c_str(),O_RDONLY)) {
			}

			~Environ() {

				if(descriptor > 0) {
					::close(descriptor);
				}
			}

			operator bool() const {
				return descriptor >= 0;
			}

			uid_t uid() {
				struct stat st;
				if(fstat(descriptor,&st) < 0)
					return (uid_t) -1;
				return st.st_uid;
			}

			int fd() {
				return this->descriptor;
			}

		};

		DBusConnection *connection = nullptr;

		// https://stackoverflow.com/questions/6496847/access-another-users-d-bus-session
		DIR * dir = opendir("/proc");
        if(!dir) {
			throw std::system_error(errno, std::system_category());
        }

        try {

			struct dirent *ent;
			while((ent=readdir(dir))!=NULL && !connection) {

				Environ environ(dir,ent->d_name);

				if(!environ || environ.uid() != uid) {
					continue;
				}

				// Check session id.
#ifdef HAVE_SYSTEMD
				if(sid && *sid) {
					char *sname = nullptr;

					// Reject pids without session.
					if(sd_pid_get_session(atoi(ent->d_name), &sname) == -ENODATA)
						continue;

					// Test if it's the required session.
					if(strcmp(sid,sname)) {
						free(sname);
						continue;
					}

					free(sname);
				}
#endif // HAVE_SYSTEMD

				// It's an user environment, scan it.
				{
					File::Text text(environ.fd());
					for(const char *ptr = text.c_str(); *ptr; ptr += (strlen(ptr)+1)) {
						if(strncmp(ptr,"DBUS_SESSION_BUS_ADDRESS",24) == 0 && ptr[24] == '=') {

							// Found session address, try to open it.

							// Get an static lock guard to avoid another change
							static mutex guard;
							lock_guard<mutex> lock(guard);

							// Save application EUID and switch to required UID.
							uid_t saved_uid = geteuid();
							if(seteuid(uid) < 0) {

								cerr << "dbus\tCan't set efective UID: " << strerror(errno) << endl;

							} else {

								DBusError err;
								dbus_error_init(&err);

								ptr += 25;

								Logger::String{"Found bus '",ptr,"' for user"}.trace("d-bus");

								connection = dbus_connection_open_private(ptr, &err);
								if(dbus_error_is_set(&err)) {

									clog << "dbus\tError '" << err.message << "' opening BUS " << ptr << endl;
									dbus_error_free(&err);
									connection = nullptr;

								} else {

									int fd = -1;
									if(dbus_connection_get_socket(connection,&fd)) {
										Logger::String("Got connection to user '",uid,"' with socket '",fd,"'").trace("d-bus");
									} else {
										Logger::String("Got connection to user '",uid,"'").trace("d-bus");
									}

								}

								// Restore to saved UID.
								seteuid(saved_uid);

							}
							break;
						}
					}
				}

			}

        } catch(...) {

			closedir(dir);
			throw;

        }

		closedir(dir);

        if(!connection) {
			throw system_error(ENOENT,system_category(),"Unable to find D-Bus session for requested user");
        }

		return connection;

	}

	static std::string get_username(uid_t uid) {

		int bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
		if (bufsize < 0)
			bufsize = 16384;

		string rc;
		char * buf = new char[bufsize];

		struct passwd pwd;
		struct passwd * result;
		if(getpwuid_r(uid, &pwd, buf, bufsize, &result)) {
			rc = "@";
			rc += std::to_string(uid);
		} else {
			rc = buf;
		}
		delete[] buf;

		return rc;

	}

	DBus::UserBus::UserBus(uid_t uid, const char *sid) : NamedBus{get_username(uid).c_str(),UserConnectionFactory(uid,sid)}, userid{uid} {

		try {
			open();
			bus_register();
		} catch(...) {

			if(conn) {
				Logger::String{"Closing connection to '",uid,"' due to initialization error"}.error("d-bus");
				dbus_connection_flush(conn);
				dbus_connection_close(conn);
				dbus_connection_unref(conn);
				conn = NULL;
			}

			throw;

		}
	}

 }


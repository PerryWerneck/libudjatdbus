/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2015 Perry Werneck <perry.werneck@gmail.com>
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

 #include <udjat/defs.h>
 #include <private/mainloop.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/handler.h>
 #include <udjat/tools/logger.h>
 #include <unistd.h>

/*---[ Implement ]----------------------------------------------------------------------------------*/

 class Context : public MainLoop::Handler {
 private:

	DBusConnection	* connection	= nullptr;
	DBusWatch		* watch			= nullptr;

 protected:
	void handle_event(const Event events) override;

 public:

	Context(DBusConnection *c, int f, DBusWatch *w, short e) : MainLoop::Handler(f,(MainLoop::Handler::Event) e), connection(c), watch(w) {
#ifdef DEBUG
		Logger::trace() << "handler\tCreating d-bus context " << hex << ((void *) this) << dec << endl;
#endif // DEBUG
	}

#ifdef DEBUG
	virtual ~Context() {
		Logger::trace() << "handler\tDestroying d-bus context " << hex << ((void *) this) << dec << endl;
	}
#endif // DEBUG

 };

 dbus_bool_t add_watch(DBusWatch *watch, DBusConnection *connection) {

	// Get event
	short event = 0;
 	unsigned int flags = dbus_watch_get_flags(watch);

	if (flags & DBUS_WATCH_READABLE)
		event |= POLLIN;

	if (flags & DBUS_WATCH_WRITABLE)
		event |= POLLOUT;

	if (flags & DBUS_WATCH_HANGUP)
		event |= POLLHUP;

	if (flags & DBUS_WATCH_ERROR)
		event |= POLLERR;

	Context *context = new Context(connection,dbus_watch_get_unix_fd(watch),watch,event);

#ifdef DEBUG
	Logger::trace() << "d-bus\t*** Adding watch " << hex << ((void *) context) << dec << " from connection " << connection << endl;
#endif // DEBUG

	dbus_watch_set_data(watch, context, NULL);

	context->enable();

	return true;
 }

 void remove_watch(DBusWatch *watch, DBusConnection *) {

	Context *context = (Context *) dbus_watch_get_data(watch);

	if(context) {

#ifdef DEBUG
		Logger::trace() << "d-bus\t*** Removing watch " << hex << ((void *) context) << dec << endl;
#endif // DEBUG

		dbus_watch_set_data(watch, NULL, NULL);
		context->disable();
		delete context;
	}

 }

 void toggle_watch(DBusWatch *watch, DBusConnection *) {

	Context *context = (Context *) dbus_watch_get_data(watch);

	if(context) {

		context->set(dbus_watch_get_unix_fd(watch));

		if (dbus_watch_get_enabled(watch)) {
#ifdef DEBUG
			Logger::trace() << "d-bus\t*** " << __FUNCTION__<< ".enable " << hex << ((void *) context) << dec << endl;
#endif // DEBUG
			context->enable();
		} else {
#ifdef DEBUG
			Logger::trace() << "d-bus\t*** " << __FUNCTION__<< ".enable " << hex << ((void *) context) << dec << endl;
#endif // DEBUG
			context->disable();
		}

	}

 }

 void Context::handle_event(const MainLoop::Handler::Event events) {

#ifdef DEBUG
	Logger::trace() << "d-bus\t*** Activity on watch " << hex << ((void *) this) << dec << " events=" << events;
#endif // DEBUG

	if(!dbus_watch_get_enabled(watch)) {
#ifdef DEBUG
		Logger::trace() << " DISABLED" << endl;
#endif // DEBUG
		disable();
		return;
	}

	unsigned int flags = 0;

	if (events & POLLIN) {
		flags |= DBUS_WATCH_READABLE;
#ifdef DEBUG
		Logger::trace() << " DBUS_WATCH_READABLE";
#endif // DEBUG
	}

	if (events & POLLOUT) {
		flags |= DBUS_WATCH_WRITABLE;
#ifdef DEBUG
		Logger::trace() << " DBUS_WATCH_WRITABLE";
#endif // DEBUG
	}

	if (events & POLLHUP) {
		flags |= DBUS_WATCH_HANGUP;
#ifdef DEBUG
		Logger::trace() << " DBUS_WATCH_HANGUP";
#endif // DEBUG
	}

	if (events & POLLERR) {
		flags |= DBUS_WATCH_ERROR;
#ifdef DEBUG
		Logger::trace() << " DBUS_WATCH_ERROR";
#endif // DEBUG
	}

#ifdef DEBUG
	Logger::trace() << endl;
#endif // DEBUG

	if(dbus_watch_handle(watch, flags) == FALSE) {
		cerr << "d-bus\tdbus_watch_handle() failed" << endl;
		return;
	}

	dbus_connection_ref(connection);
	while (dbus_connection_get_dispatch_status(connection) == DBUS_DISPATCH_DATA_REMAINS)
        dbus_connection_dispatch(connection);
	dbus_connection_unref(connection);

 }

 void Abstract::DBus::Connection::bind() {

	lock_guard<mutex> lock(guard);

	// Keep running if d-bus disconnect.
	dbus_connection_set_exit_on_disconnect(conn, false);

	// Initialize Main loop.
	MainLoop::getInstance();

	// Set watch functions.
	if(!dbus_connection_set_watch_functions(
		conn,
		(DBusAddWatchFunction) add_watch,
		(DBusRemoveWatchFunction) remove_watch,
		(DBusWatchToggledFunction) toggle_watch,
		conn,
		nullptr)
	) {
		throw runtime_error("dbus_connection_set_watch_functions has failed");
	}

	// Set timeout functions.
	if(!dbus_connection_set_timeout_functions(
		conn,
		(DBusAddTimeoutFunction) add_timeout,
		(DBusRemoveTimeoutFunction) remove_timeout,
		(DBusTimeoutToggledFunction) toggle_timeout,
		conn,
		nullptr)
	) {
		throw runtime_error("dbus_connection_set_timeout_functions has failed");
	}
 }

 void Abstract::DBus::Connection::unbind() {

	lock_guard<mutex> lock(guard);

	if(!dbus_connection_set_watch_functions(
		conn,
		(DBusAddWatchFunction) NULL,
		(DBusRemoveWatchFunction) NULL,
		(DBusWatchToggledFunction) NULL,
		NULL,
		nullptr)
	) {
		Logger::String{"dbus_connection_set_watch_functions failed"}.error("d-bus");
	}

	if(!dbus_connection_set_timeout_functions(
		conn,
		(DBusAddTimeoutFunction) NULL,
		(DBusRemoveTimeoutFunction) NULL,
		(DBusTimeoutToggledFunction) NULL,
		NULL,
		nullptr)
	) {
		Logger::String{"dbus_connection_set_timeout_functions failed"}.error("d-bus");
	}

 }



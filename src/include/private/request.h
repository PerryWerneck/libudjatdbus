/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2026 Perry Werneck <perry.werneck@gmail.com>
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

 #pragma once

 #include <udjat/defs.h>
 #include <udjat/tools/request.h>
 #include <udjat/tools/interface.h>
 #include <udjat/tools/http/method.h>
 #include <dbus/dbus.h>
 #include <functional>

 namespace Udjat::DBus {
 
	/// @brief Convenience method to enumerate methods
	UDJAT_PRIVATE bool for_each(const std::function<bool(const HTTP::Method http, const char *dbus)> &method) noexcept;

	class UDJAT_PRIVATE Request : public Udjat::Request {
	private:
		DBusMessage *message;

	public:
		Request(DBusMessage *message);
		~Request() override;

		HTTP::Method method() const noexcept override;

		/// @brief Parse input values from message.
		/// @param intf The interface for input schema.
		/// @param message The DBus message with inputs.
		/// @return Error message.
		/// @retval nullptr if the inputs were parsed.
		DBusMessage * parse_input(const Udjat::Interface &intf);

	};

 }


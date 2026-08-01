/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 202 Perry Werneck <perry.werneck@gmail.com>
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
 #include <config.h>
 #include <udjat/defs.h>
 #include <dbus/dbus.h>
 #include <udjat/tools/http/method.h>
 #include <udjat/tools/schema.h>

 namespace Udjat {

	namespace DBus {

		/// @brief Get dbus-method from http name.
		/// @param http The http method name.
		/// @return The d-bus method name.
		/// @retval nullptr Name was not found.
		UDJAT_PRIVATE const char * MethodNameFactory(const HTTP::Method http) noexcept;

		UDJAT_PRIVATE bool for_each(const std::function<bool(const HTTP::Method http, const char *dbus)> &callback) noexcept;

		UDJAT_PRIVATE const char * StringTypeFactory(const Schema::Type type) noexcept;

		/// @brief Extract dbus basic value from variant.
		/// @param schema The basic value definition.
		/// @param value The variant to convert
		/// @param arg_type The dbus argument type found.
		/// @param dval The dbus basic value found.
		/// @return Convertion result.
		/// @retval true The value was converted.
		/// @retval false Unable to convert value.
		UDJAT_PRIVATE bool ValueFactory(const Udjat::Variant &value, const Schema::Item &schema, int &arg_type, DBusBasicValue &dval) noexcept;

	}

	namespace HTTP {

		UDJAT_PRIVATE HTTP::Method MethodFactory(DBusMessage *message);

	}


 }
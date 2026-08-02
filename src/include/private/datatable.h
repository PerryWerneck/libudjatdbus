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
 #include <udjat/tools/datatable.h>
 #include <dbus/dbus.h>

 namespace Udjat::DBus {

	/// @brief Abstract object containing values ordered in rows & columns.
	class UDJAT_API DataTable : public Udjat::DataTable {
	private:
		DBusMessage *reply;
		DBusMessageIter iter, array;

	public:
		DataTable(DBusMessage *request, const OutputSchema &schema);
		~DataTable() override;

		/// @brief Add one row to the table.
		/// @param value Object with column data to extract based on schema.
		Udjat::DataTable & push_back(const Value &row) override;

		/// @brief Build output message.
		/// @return Response message.
		DBusMessage * MessageFactory();

	};

 }


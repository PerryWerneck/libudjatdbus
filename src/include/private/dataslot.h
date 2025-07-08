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
  * @brief Declares singleton for data slot.
  */

 #pragma once

 #include <config.h>
 #include <udjat/defs.h>
 #include <dbus/dbus.h>
 #include <udjat/tools/logger.h>

 class DataSlot {
 private:
	dbus_int32_t slot = -1; // The passed-in slot must be initialized to -1, and is filled in with the slot ID
	DataSlot() {
		dbus_connection_allocate_data_slot(&slot);
		Udjat::Logger::String{"Got slot '",slot,"' for connection watchdog"}.write(Udjat::Logger::Debug,"d-bus");
	}

 public:

	~DataSlot() {
		dbus_connection_free_data_slot(&slot);
	}

	static DataSlot & getInstance() {
		static DataSlot instance;
		return instance;
	}

	inline dbus_int32_t value() const noexcept {
		return slot;
	}

 };


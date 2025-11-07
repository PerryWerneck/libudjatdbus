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
  * @brief Declare d-bus alerts.
  */

 #pragma once

 #include <udjat/defs.h>
 #include <udjat/alert.h>
 #include <dbus/dbus.h>
 #include <udjat/tools/abstract/object.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/dbus/emitter.h>
 #include <vector>

 namespace Udjat {

	namespace DBus {

		class UDJAT_API Alert : public Udjat::Alert, private Emitter {
		protected:
			int emit() override;
			void reset(time_t next) noexcept override;

		public:

			class UDJAT_API Factory : public Udjat::Alert::Factory {
			public:
				Factory(const char *name = "dbus");
				virtual ~Factory();
				std::shared_ptr<Udjat::Alert> AlertFactory(const Abstract::Object &parent, const XML::Node &node) const override;

			};

			Alert(const XML::Node &node);
			virtual ~Alert();

			bool activate() noexcept override;
			bool activate(const Abstract::Object &object) noexcept override;

		};

	}

 }

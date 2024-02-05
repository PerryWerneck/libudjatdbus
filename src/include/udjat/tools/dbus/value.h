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
  * @brief Declares D-Bus Value.
  */

 #pragma once
 #include <udjat/defs.h>
 #include <dbus/dbus.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/dbus/defs.h>
 #include <map>

 namespace Udjat {

 	namespace DBus {


 		/// @brief D-Bus Value
		class UDJAT_API Value : public Udjat::Value {
		private:

			/// @brief D-Bus data type.
			int type;

			/// @brief D-Bus value.
			DBusBasicValue value;

			/// @brief Value children.
			std::map<std::string,Value *> children;

			/// @brief Check if the value dont have a signature.
			/// @return true if the value can be added on signature.
			inline bool noSignature() const noexcept {
				return (type == DBUS_TYPE_INVALID || type == DBUS_TYPE_ARRAY || type == DBUS_TYPE_DICT_ENTRY);
			}

			/// @brief Get signature for array export.
			std::string getArraySignature() const noexcept;

		public:

			// String values have an strdup; the copy can invalidate the pointer.
			Value(const Value *src);
			Value(const Value &src);
			Value(Message &message);

			Value();
			Value(int type, const char *value = nullptr);
			virtual ~Value();

			/// @brief Add value on iter.
			void get(DBusMessageIter *iter) const;

			/// @brief Set value from iter.
			/// @return true if the value is valid.
			bool set(DBusMessageIter *iter);

			/// @brief The value has children?
			inline bool empty() const noexcept {
				return children.empty();
			}

			inline bool operator==(int type) const noexcept {
				return this->type == type;
			}

			Udjat::Value & reset(const Udjat::Value::Type type = Udjat::Value::Undefined) override;

			bool isNull() const override;

			Udjat::Value & operator[](const char *name) override;

			Udjat::Value & append(const Type type) override;
			Udjat::Value & set(const Udjat::Value &value) override;

			Udjat::Value & set(const char *value, const Type type) override;
			Udjat::Value & set(const short value) override;
			Udjat::Value & set(const unsigned short value) override;
			Udjat::Value & set(const int value) override;
			Udjat::Value & set(const unsigned int value) override;
			Udjat::Value & set(const long value) override;
			Udjat::Value & set(const unsigned long value) override;
			Udjat::Value & set(const TimeStamp value) override;
			Udjat::Value & set(const bool value) override;
			Udjat::Value & set(const float value) override;
			Udjat::Value & set(const double value) override;

			const Udjat::Value & get(std::string &value) const override;
			const Udjat::Value & get(short &value) const override;
			const Udjat::Value & get(unsigned short &value) const override;
			const Udjat::Value & get(int &value) const override;
			const Udjat::Value & get(unsigned int &value) const override;
			const Udjat::Value & get(long &value) const override;
			const Udjat::Value & get(unsigned long &value) const override;
			const Udjat::Value & get(TimeStamp &value) const override;
			const Udjat::Value & get(bool &value) const override;
			const Udjat::Value & get(float &value) const override;
			const Udjat::Value & get(double &value) const override;

			int getFD() const;

		};

 	}

 }

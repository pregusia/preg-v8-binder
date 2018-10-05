/*
 * Copyright (c) 2010-2015 OTClient <https://github.com/edubart/otclient>
 * Modded by pregusia
 *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#ifndef STDEXT_SHARED_OBJECT_H
#define STDEXT_SHARED_OBJECT_H

#include "types.h"
#include "format.h"
#include "demangle.h"
#include <type_traits>
#include <functional>
#include <ostream>
#include <cassert>

namespace stdext {

	template<class T>
	class object_ptr;

	class object {
		public:
			object();
			virtual ~object();

			virtual std::string toString() const { return format("[Object %s %08X]", typeid(this).name(), (void*)this); }

			template<typename T> stdext::object_ptr<T> static_self_cast() { return stdext::object_ptr<T>(static_cast<T*>(this)); }
			template<typename T> stdext::object_ptr<T> dynamic_self_cast() { return stdext::object_ptr<T>(dynamic_cast<T*>(this)); }
			template<typename T> stdext::object_ptr<T> const_self_cast() { return stdext::object_ptr<T>(const_cast<T*>(this)); }

			int32_t __refsCount() const { return m_objectRefs; }
			uint64_t __objectId() const { return m_objectId; }

			virtual void __refsDec() {
				assert(m_objectRefs > 0);
				m_objectRefs -= 1;
				if (m_objectRefs == 0) delete this;
			}
			virtual void __refsInc() {
				m_objectRefs += 1;
			}

		private:
			volatile int32_t m_objectRefs;
			uint64_t m_objectId;

			object(const object& from);
			object& operator=(const object& from);
	};


	template<class T>
	class object_ptr {
		public:
			typedef T element_type;

			object_ptr(): px(nullptr) { }
			object_ptr(T* p) : px(p) {
				static_assert(std::is_base_of<object, T>::value, "classes using stdext::object_ptr must be a derived of stdext::object");
				refsInc();
			}
			object_ptr(const object_ptr& rhs): px(rhs.px) { refsInc(); }

			template<class U>
			object_ptr(object_ptr<U> const& rhs, typename std::is_convertible<U,T>::type* = nullptr) : px(rhs.get()) { refsInc(); }

			~object_ptr() { refsDec(); }


			void reset() { object_ptr().swap(*this); }
			void reset(T* rhs) { object_ptr(rhs).swap(*this); }
			void swap(object_ptr& rhs) { std::swap(px, rhs.px); }
			bool empty() const { return px == nullptr; }
			T* get() const { return px; }

			T& operator*() const {
				assert(px != nullptr);
				return *px;
			}
			T* operator->() const {
				assert(px != nullptr);
				return px;
			}

			object_ptr& operator=(const object_ptr& rhs) { object_ptr(rhs).swap(*this); return *this; }
			object_ptr& operator=(T* rhs) { object_ptr(rhs).swap(*this); return *this; }

			// implicit conversion to bool
			typedef T* object_ptr::*unspecified_bool_type;
			operator unspecified_bool_type() const { return px == nullptr ? nullptr : &object_ptr::px; }
			bool operator!() const { return px == nullptr; }

			// std::move support
			object_ptr(object_ptr&& rhs) noexcept: px(rhs.px) { rhs.px = nullptr; }
			object_ptr& operator=(object_ptr&& rhs) {
				object_ptr(static_cast<object_ptr&&>(rhs)).swap(*this);
				return *this;
			}

		private:
			void refsInc() {
				if (px != nullptr) {
					dynamic_cast<object*>(px)->__refsInc();
				}
			}

			void refsDec() {
				if (px != nullptr) {
					dynamic_cast<object*>(px)->__refsDec();
				}
			}

			T* px;
	};

	template<class T, class U> bool operator==(object_ptr<T> const& a, object_ptr<U> const& b) { return a.get() == b.get(); }
	template<class T, class U> bool operator!=(object_ptr<T> const& a, object_ptr<U> const& b) { return a.get() != b.get(); }
	template<class T, class U> bool operator==(object_ptr<T> const& a, U* b) { return a.get() == b; }
	template<class T, class U> bool operator!=(object_ptr<T> const& a, U* b) { return a.get() != b; }
	template<class T, class U> bool operator==(T * a, object_ptr<U> const& b) { return a == b.get(); }
	template<class T, class U> bool operator!=(T * a, object_ptr<U> const& b) { return a != b.get(); }
	template<class T> bool operator<(object_ptr<T> const& a, object_ptr<T> const& b) { return std::less<T*>()(a.get(), b.get()); }


	// operator<< support
	template<class E, class T, class Y> std::basic_ostream<E, T>& operator<<(std::basic_ostream<E, T>& os, object_ptr<Y> const& p) {
		if (p.empty()) {
			os << "null";
		} else {
			os << p.get()->toString();
		}
		return os;
	}

}

namespace std {

	// hash, for unordered_map support
	template<typename T>
	struct hash<stdext::object_ptr<T>> {
		size_t operator()(const stdext::object_ptr<T>& p) const { return std::hash<T*>()(p.get()); }
	};

	// swap support
	template<class T>
	void swap(stdext::object_ptr<T>& lhs, stdext::object_ptr<T>& rhs) {
		lhs.swap(rhs);
	}

}

#endif

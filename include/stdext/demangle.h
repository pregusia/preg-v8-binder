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


#ifndef STDEXT_DEMANGLE_H
#define STDEXT_DEMANGLE_H

#include <typeinfo>
#include <vector>
#include <string>

namespace stdext {

	class demangled_name {
		public:
			const std::string& last() const;
			std::string full() const;
			bool empty() const { return m_parts.empty(); }

			static demangled_name createFromString(const char* name);

			template<typename T>
			static demangled_name get() {
				return createFromString(typeid(T).name());
			}

			bool operator==(const demangled_name& n) const { return n.full() == full(); }

		private:
			std::vector<std::string> m_parts;
	};

}

#endif

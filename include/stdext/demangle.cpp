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


#include "demangle.h"
#include <cxxabi.h>
#include <cstring>
#include <cstdlib>
#include <sstream>

namespace stdext {

	const std::string& demangled_name::last() const {
		if (m_parts.empty()) {
			static std::string empty;
			return empty;
		} else {
			return m_parts.back();
		}
	}

	std::string demangled_name::full() const {
		std::stringstream ss;
		bool first = true;

		for(auto it=m_parts.begin();it != m_parts.end();++it) {
			if (!first) {
				ss << "::";
			}
			ss << *it;
			first = false;
		}

		return ss.str();
	}

	demangled_name demangled_name::createFromString(const char* name) {
		size_t len;
		int status;
		char* buf = abi::__cxa_demangle(name, NULL, &len, &status);
		demangled_name res;

		if (buf != NULL) {
			std::string str(buf);
			size_t pos = 0;
			std::string token;
			std::string delimiter = "::";

			while((pos = str.find(delimiter)) != std::string::npos) {
			    std::string token = str.substr(0, pos);
			    if (!token.empty()) {
			    	res.m_parts.push_back(token);
			    }
			    str.erase(0, pos + delimiter.length());
			}

			if (!str.empty()) {
				res.m_parts.push_back(str);
			}

			free(buf);
		}

		return res;
	}

}

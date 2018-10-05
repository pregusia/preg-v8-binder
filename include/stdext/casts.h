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


#ifndef INCLUDE_STDEXT_CASTS_H_
#define INCLUDE_STDEXT_CASTS_H_

#include <string>
#include <sstream>

namespace stdext {

	// ************************************************************************************
	template<typename T, typename R>
	inline bool cast(const T& in, R& out) {
		std::stringstream ss;
		ss << in;
		ss >> out;
		return !!ss && ss.eof();
	}

	// ************************************************************************************
	template<typename T>
	inline bool cast(const T& in, std::string& out) {
		std::stringstream ss;
		ss << in;
		out = ss.str();
		return true;
	}

	// ************************************************************************************
	template<>
	inline bool cast(const std::string& in, std::string& out) {
		out = in;
		return true;
	}

	// ************************************************************************************
	template<>
	inline bool cast(const std::string& in, bool& b) {
		if(in == "true" || in == "1" || in == "on")
			b = true;
		else if(in == "false" || in == "0" || in == "off")
			b = false;
		else
			return false;
		return true;
	}

	// ************************************************************************************
	template<>
	inline bool cast(const std::string& in, char& c) {
		if(in.length() != 1) return false;
		c = in[0];
		return true;
	}

	// ************************************************************************************
	template<>
	inline bool cast(const std::string& in, long& l) {
		if(in.find_first_not_of("-0123456789") != std::string::npos)
			return false;
		std::size_t t = in.find_last_of('-');
		if(t != std::string::npos && t != 0)
			return false;
		l = atol(in.c_str());
		return true;
	}

	// ************************************************************************************
	template<>
	inline bool cast(const std::string& in, int& i) {
		long l;
		if(cast(in, l)) {
			i=l;
			return true;
		}
		return false;
	}

	// ************************************************************************************
	template<>
	inline bool cast(const std::string& in, double& d) {
		if(in.find_first_not_of("-0123456789.") != std::string::npos)
			return false;
		std::size_t t = in.find_last_of('-');
		if(t != std::string::npos &&  t != 0)
			return false;
		t = in.find_first_of('.');
		if(t != std::string::npos && (t == 0 || t == in.length()-1 || in.find_first_of('.', t+1) != std::string::npos))
			return false;
		d = atof(in.c_str());
		return true;
	}

	// ************************************************************************************
	template<>
	inline bool cast(const std::string& in, float& f) {
		double d;
		if(cast(in, d)) {
			f=(float)d;
			return true;
		}
		return false;
	}

	// ************************************************************************************
	template<>
	inline bool cast(const bool& in, std::string& out) {
		out = (in ? "true" : "false");
		return true;
	}

	// ************************************************************************************
	template<typename R, typename T>
	inline R safeCast(const T& t) {
		R r = R();
		if(!cast(t, r)) {
			// TODO: throw some exception?
		}
		return r;
	}

	// ************************************************************************************
	template<typename R, typename T>
	inline R unsafeCast(const T& t, R def = R()) {
		R r = def;
		cast(t, r);
		return r;
	}

	// ************************************************************************************
	template<typename R, typename T>
	inline R bitsCast(const T& val) {
		static_assert(sizeof(R) == sizeof(T), "diff sizes");
		union {
			R r;
			T t;
		} u;
		u.t = val;
		return u.r;
	}

}



#endif /* INCLUDE_STDEXT_CASTS_H_ */

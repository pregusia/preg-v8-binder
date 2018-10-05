/*
 * Copyright (c) preg-v8-binder <https://github.com/pregusia/preg-v8-binder>
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


#ifndef INCLUDE_SCRIPTING_CONVERTERS_H_
#define INCLUDE_SCRIPTING_CONVERTERS_H_

#include "base.h"
#include <v8.h>

namespace scripting { class Engine; }

namespace scripting { namespace converters {

	v8::Local<v8::Value> convertTo(Engine* engine, bool v);
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, bool& out);

	v8::Local<v8::Value> convertTo(Engine* engine, int64_t v);
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, int64_t& out);

	v8::Local<v8::Value> convertTo(Engine* engine, uint64_t v);
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, uint64_t& out);

	v8::Local<v8::Value> convertTo(Engine* engine, int32_t v);
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, int32_t& out);

	v8::Local<v8::Value> convertTo(Engine* engine, uint32_t v);
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, uint32_t& out);

	v8::Local<v8::Value> convertTo(Engine* engine, int16_t v);
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, int16_t& out);

	v8::Local<v8::Value> convertTo(Engine* engine, uint16_t v);
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, uint16_t& out);

	v8::Local<v8::Value> convertTo(Engine* engine, int8_t v);
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, int8_t& out);

	v8::Local<v8::Value> convertTo(Engine* engine, uint8_t v);
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, uint8_t& out);

	v8::Local<v8::Value> convertTo(Engine* engine, float v);
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, float& out);

	v8::Local<v8::Value> convertTo(Engine* engine, double v);
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, double& out);

	v8::Local<v8::Value> convertTo(Engine* engine, const std::string& v);
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, std::string& out);

	v8::Local<v8::Value> convertTo(Engine* engine, const v8::Local<v8::Value>& v);
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, v8::Local<v8::Value>& out);

	v8::Local<v8::Value> convertTo(Engine* engine, const v8::Local<v8::Object>& v);
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, v8::Local<v8::Object>& out);



	template<typename T>
	v8::Local<v8::Value> convertTo(Engine* engine, const stdext::object_ptr<T>& v);

	template<typename T>
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, stdext::object_ptr<T>& out);

	template<typename T>
	v8::Local<v8::Value> convertTo(Engine* engine, const std::vector<T>& v);
	template<typename T>
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, std::vector<T>& out);

	template<typename T>
	v8::Local<v8::Value> convertTo(Engine* engine, const std::list<T>& v);

	template<typename T>
	v8::Local<v8::Value> convertTo(Engine* engine, const std::deque<T>& v);

	template<typename T>
	v8::Local<v8::Value> convertTo(Engine* engine, const std::map<int, T>& v);

	template<typename T>
	v8::Local<v8::Value> convertTo(Engine* engine, const std::map<std::string, T>& v);

	template<typename R, typename... Args>
	v8::Local<v8::Value> convertTo(Engine* engine, const std::function<R(Args...)>& f);

	template<typename R, typename... Args>
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, std::function<R(Args...)>& out);

	template<typename T,class=typename std::enable_if<std::is_enum<T>::value>::type>
	v8::Local<v8::Value> convertTo(Engine* engine, const T& v);

	template<typename T,class=typename std::enable_if<std::is_enum<T>::value>::type>
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, T& out);

	template<typename T>
	struct ConverterHelper {
		static T from(Engine* engine, v8::Local<v8::Value> v) {
			T res = T();
			convertFrom(engine, v, res);
			return res;
		}
		static v8::Local<v8::Value> to(Engine* engine, const T& v) {
			return convertTo(engine, v);
		}
	};

	template<>
	struct ConverterHelper<void> {
		static void from(Engine* engine, v8::Local<v8::Value> v) { }
	};

} }

#endif /* INCLUDE_SCRIPTING_CONVERTERS_H_ */

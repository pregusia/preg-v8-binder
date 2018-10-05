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


#include "converters.h"
#include "engine.h"

namespace scripting { namespace converters {

	// ************************************************************************************
	v8::Local<v8::Value> convertTo(Engine* engine, bool v) { return engine->newBoolean(v); }
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, bool& out) { out = v.IsEmpty() ? false : v->ToBoolean()->Value(); }

	// ************************************************************************************
	v8::Local<v8::Value> convertTo(Engine* engine, int64_t v) { return engine->newInt(v); }
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, int64_t& out) { out = v.IsEmpty() ? 0 : v->ToInt32()->Value(); }

	// ************************************************************************************
	v8::Local<v8::Value> convertTo(Engine* engine, uint64_t v) { return engine->newInt(v); }
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, uint64_t& out) { out = v.IsEmpty() ? 0 : v->ToUint32()->Value(); }

	// ************************************************************************************
	v8::Local<v8::Value> convertTo(Engine* engine, uint32_t v) { return engine->newInt(v); }
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, uint32_t& out) { out = v.IsEmpty() ? 0 : v->ToUint32()->Value(); }

	// ************************************************************************************
	v8::Local<v8::Value> convertTo(Engine* engine, int32_t v) { return engine->newInt(v); }
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, int32_t& out) { out = v.IsEmpty() ? 0 : v->ToInt32()->Value(); }

	// ************************************************************************************
	v8::Local<v8::Value> convertTo(Engine* engine, uint16_t v) { return engine->newInt(v); }
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, uint16_t& out) { out = v.IsEmpty() ? 0 : v->ToUint32()->Value(); }

	// ************************************************************************************
	v8::Local<v8::Value> convertTo(Engine* engine, int16_t v) { return engine->newInt(v); }
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, int16_t& out) { out = v.IsEmpty() ? 0 : v->ToInt32()->Value(); }

	// ************************************************************************************
	v8::Local<v8::Value> convertTo(Engine* engine, uint8_t v) { return engine->newInt(v); }
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, uint8_t& out) { out = v.IsEmpty() ? 0 : v->ToUint32()->Value(); }

	// ************************************************************************************
	v8::Local<v8::Value> convertTo(Engine* engine, int8_t v) { return engine->newInt(v); }
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, int8_t& out) { out = v.IsEmpty() ? 0 : v->ToInt32()->Value(); }

	// ************************************************************************************
	v8::Local<v8::Value> convertTo(Engine* engine, float v) { return engine->newFloat(v); }
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, float& out) { out = v.IsEmpty() ? 0 : v->ToNumber()->NumberValue(); }

	// ************************************************************************************
	v8::Local<v8::Value> convertTo(Engine* engine, double v) { return engine->newFloat(v); }
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, double& out) { out = v.IsEmpty() ? 0 : v->ToNumber()->NumberValue(); }


	// ************************************************************************************
	v8::Local<v8::Value> convertTo(Engine* engine, const std::string& v) {
		return engine->newString(v);
	}

	// ************************************************************************************
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, std::string& out) {
		if (v.IsEmpty()) {
			out = "";
		} else {
			v8::String::Utf8Value w(v);
			if (*w != nullptr) {
				out = *w;
			} else {
				out = "Conversion failed";
			}
		}
	}

	// ************************************************************************************
	v8::Local<v8::Value> convertTo(Engine* engine, const v8::Local<v8::Value>& v) {
		return v;
	}

	// ************************************************************************************
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, v8::Local<v8::Value>& out) {
		out = v;
	}

	// ************************************************************************************
	v8::Local<v8::Value> convertTo(Engine* engine, const v8::Local<v8::Object>& v) {
		return v;
	}

	// ************************************************************************************
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, v8::Local<v8::Object>& out) {
		if (!v.IsEmpty() && v->IsObject()) {
			out = v8::Local<v8::Object>::Cast(v);
		}
	}


} }

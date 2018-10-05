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


#include "engine.h"
#include "utils.h"

namespace scripting { namespace internal {

	// ************************************************************************************
	bool SetObjectPropChain(Engine* engine, v8::Local<v8::Object> obj, const std::string& name, const v8::Local<v8::Value>& val) {
		if (name.empty()) return false;
		auto arr = utils::split(name,'.');
		for(size_t i=0;i<arr.size();++i) {
			v8::Local<v8::String> str = engine->newString(arr[i]);

			if (i == arr.size() - 1) {
				// last element
				obj->Set(str, val);
				return true;
			} else {
				// sth between, need to create new object it not exists
				if (!obj->Has(str)) {
					obj->Set(str, v8::Object::New(engine->isolate()));
				}

				v8::Local<v8::Value> tmpVal = obj->Get(str);
				if (!tmpVal.IsEmpty()) {
					if (tmpVal->IsObject()) {
						obj = v8::Local<v8::Object>::Cast(tmpVal);
					} else {
						return false;
					}
				}
			}
		}

		return false;
	}

	// ************************************************************************************
	std::string normalizePrototypeName(const std::string& name, const std::string& ns) {
		if (ns.empty()) {
			return name;
		} else {
			std::string tmp = ns;
			if (tmp.back() != '.') tmp = tmp + ".";
			tmp = tmp + name;
			return tmp;
		}
	}

	// ************************************************************************************
	void callCtorEvent(Engine* engine, v8::Local<v8::Object>& obj, const v8::FunctionCallbackInfo<v8::Value>& args) {
		v8::Local<v8::Value> dollarObjV = engine->context()->Global()->Get(engine->newString("$"));
		if (!dollarObjV.IsEmpty() && dollarObjV->IsFunction()) {
			v8::Local<v8::Function> dollarObj = v8::Local<v8::Function>::Cast(dollarObjV);
			v8::Local<v8::Function> eventCallStaticReverseFunc = v8::Local<v8::Function>::Cast(dollarObj->Get(engine->newString("eventCallStaticReverse")));

			v8::Local<v8::Value> arr[16];
			arr[0] = obj;
			arr[1] = engine->newString("ctor");
			for(int i=0;i<args.Length();++i) arr[2 + i] = args[i];

			eventCallStaticReverseFunc->CallAsFunction(engine->context(), dollarObj, args.Length() + 2, arr);
		}
	}

} }


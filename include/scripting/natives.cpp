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


#include "natives.h"
#include "engine.h"
#include "utils.h"


namespace scripting {

	// ************************************************************************************
	void NativeFunctions::RegisterGlobalFunctions(Engine* engine, v8::Local<v8::ObjectTemplate> global) {
		global->Set(engine->newString("print"),v8::FunctionTemplate::New(engine->isolate(), Print, engine->newExternal(engine)));
		global->Set(engine->newString("extend"),v8::FunctionTemplate::New(engine->isolate(), Extend, engine->newExternal(engine)));
		global->Set(engine->newString("ensureGlobalObject"),v8::FunctionTemplate::New(engine->isolate(), EnsureGlobalObject, engine->newExternal(engine)));
	}

	// ************************************************************************************
	void NativeFunctions::RegisterObjectsFunctions(Engine* engine) {
		auto stringObj = engine->getGlobalObject("String");
		stringObj->Set(engine->newString("format"),v8::FunctionTemplate::New(engine->isolate(), StringFormat, engine->newExternal(engine))->GetFunction());
	}

	// ************************************************************************************
	void NativeFunctions::Print(const v8::FunctionCallbackInfo<v8::Value>& args) {
		bool first = true;
		std::stringstream ss;
		for (int i = 0; i < args.Length(); i++) {
			if (first) first = false;
			else ss << " ";

			v8::String::Utf8Value str(args[i]);
			ss << *str;
		}

		utils::logScript(ss.str());
	}

	// ************************************************************************************
	void NativeFunctions::StringFormat(const v8::FunctionCallbackInfo<v8::Value>& args) {
		Engine* engine = (Engine*)v8::Local<v8::External>::Cast(args.Data())->Value();

		// string.format(fmt, ...) <- like c format

		if (args.Length() == 0) {
			args.GetReturnValue().Set(engine->newString(""));
			return;
		}

		if (args.Length() == 1) {
			args.GetReturnValue().Set(args[0]);
			return;
		}

		std::stringstream ss;
		std::string format = converters::ConverterHelper<std::string>::from(engine, args[0]);
		std::string::size_type pos = 0;
		int32_t argNr = 1;

		if (format.empty()) {
			args.GetReturnValue().Set(engine->newString(""));
			return;
		}

		auto captureFormat = [&]() -> std::string {
			char buf[1024] = { 0 };
			buf[0] = '%';
			int32_t p = 1;
			while(pos < format.size()) {
				char ch = format[pos++];
				buf[p++] = ch;
				if (ch == 'd' || ch == 's' || ch == '%' || ch == 'f') break;
			}
			return buf;
		};

		while(pos < format.size()) {
			if (format[pos] == '%') {
				pos += 1;
				auto fmt = captureFormat();
				if (fmt.back() == '%') {
					ss.put('%');
					continue;
				}
				if (fmt.back() == 'd') {
					int32_t v = (argNr < args.Length()) ? converters::ConverterHelper<int32_t>::from(engine, args[argNr]) : 0;
					ss << stdext::format(fmt, v);
					argNr += 1;
					continue;
				}
				if (fmt.back() == 'f') {
					float v = (argNr < args.Length()) ? converters::ConverterHelper<float>::from(engine, args[argNr]) : 0;
					ss << stdext::format(fmt, v);
					argNr += 1;
					continue;
				}
				if (fmt.back() == 's') {
					std::string v = (argNr < args.Length()) ? converters::ConverterHelper<std::string>::from(engine, args[argNr]) : "";
					ss << stdext::format(fmt, v);
					argNr += 1;
					continue;
				}

				ss << "??";
				argNr += 1;
				continue;
			} else {
				ss.put(format[pos]);
				pos += 1;
			}
		}

		args.GetReturnValue().Set(engine->newString(ss.str()));
	}


	// ************************************************************************************
	void NativeFunctions::Extend(const v8::FunctionCallbackInfo<v8::Value>& args) {
		Engine* engine = (Engine*)v8::Local<v8::External>::Cast(args.Data())->Value();
		if (args.Length() < 2) return;

		std::string baseClass = converters::ConverterHelper<std::string>::from(engine, args[0]);
		std::string className = converters::ConverterHelper<std::string>::from(engine, args[1]);

		engine->extendPrototype(className, baseClass);
	}

	// ************************************************************************************
	void NativeFunctions::EnsureGlobalObject(const v8::FunctionCallbackInfo<v8::Value>& args) {
		Engine* engine = (Engine*)v8::Local<v8::External>::Cast(args.Data())->Value();
		if (args.Length() < 1) return;

		std::string name = converters::ConverterHelper<std::string>::from(engine, args[0]);
		auto arr = utils::split(name, '.');

		v8::Local<v8::Object> obj = engine->getGlobalObject();
		for(size_t i=0;i<arr.size();++i) {
			v8::Local<v8::String> str = engine->newString(arr[i]);

			if (!obj->Has(str)) {
				obj->Set(str, v8::Object::New(engine->isolate()));
			}

			v8::Local<v8::Value> tmpVal = obj->Get(str);
			if (!tmpVal.IsEmpty()) {
				if (tmpVal->IsObject()) {
					obj = v8::Local<v8::Object>::Cast(tmpVal);
				} else {
					return;
				}
			}
		}
	}


} /* namespace scripting */


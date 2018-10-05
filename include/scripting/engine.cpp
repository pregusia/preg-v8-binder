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
#include <v8.h>
#include <libplatform/libplatform.h>
#include "natives.h"

scripting::Engine* g_engineScripting = nullptr;

namespace scripting {

	class ScriptingAllocator : public v8::ArrayBuffer::Allocator {
		public:
			virtual void* Allocate(size_t length) {
				void* data = AllocateUninitialized(length);
				return data == NULL ? data : memset(data, 0, length);
			}

			virtual void* AllocateUninitialized(size_t length) { return malloc(length); }

			virtual void Free(void* data, size_t) { free(data); }

	};



	// ************************************************************************************
	void ScriptingScope::checkThrowException(bool clear) {
		if (m_tryCatch.HasCaught()) {
			std::stringstream ss;
			ss << "ScriptingException occured" << std::endl;

			v8::Local<v8::Message> msg = m_tryCatch.Message();
			if (!msg.IsEmpty()) {
				ss << "Line: " << msg->GetLineNumber() << std::endl;
				ss << "File: " << converters::ConverterHelper<std::string>::from(m_engine, msg->GetScriptOrigin().ResourceName()) << std::endl;
			}

			ss << converters::ConverterHelper<std::string>::from(m_engine, m_tryCatch.StackTrace());

			if (clear) {
				m_tryCatch.Reset();
			}

			throw ScriptingException(ss.str());
		}
	}

	// ************************************************************************************
	bool ScriptingScope::wasException(bool clear) {
		bool res = m_tryCatch.HasCaught();
		if (clear) {
			m_tryCatch.Reset();
		}
		return res;
	}



	// ************************************************************************************
	Engine::Engine() {
		v8::V8::InitializeICU();

		m_platform = v8::platform::CreateDefaultPlatform();
		v8::V8::InitializePlatform(m_platform);
		v8::V8::Initialize();

		v8::Isolate::CreateParams isolateCreateParams;
		isolateCreateParams.array_buffer_allocator = new ScriptingAllocator();

		m_isolate = v8::Isolate::New(isolateCreateParams);
		m_isolate->SetData(0, this);
		m_suppressCtorCallback = false;

		if (true) {
			v8::Isolate::Scope isolateScope(m_isolate);
			v8::HandleScope handleScope(m_isolate);

			// creating context
			v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(m_isolate);

			NativeFunctions::RegisterGlobalFunctions(this, global);

			v8::Local<v8::Context> context = v8::Context::New(m_isolate, nullptr, global);
			m_context.Reset(m_isolate, context);
		}

		if (true) {
			ScriptingScope scope(this);

			auto nativeClassName = stdext::demangled_name::get<ScriptableObject>();
			registerPrototype("ScriptableObject", "", nativeClassName, &functions::ConstructorCallback<ScriptableObject>);

			registerNativeClassMemberFunction<ScriptableObject>("eventRegister", &ScriptableObject::eventRegister);
			registerNativeClassMemberFunction<ScriptableObject>("eventUnregister", &ScriptableObject::eventUnregister);
		}

		if (true) {
			// core script
			runString("core.js",CORE_SCRIPT);
		}

		if (true) {
			ScriptingScope scope(this);
			NativeFunctions::RegisterObjectsFunctions(this);
		}
	}

	// ************************************************************************************
	Engine::~Engine() {
		m_isolate->Dispose();
		v8::V8::Dispose();
		v8::V8::ShutdownPlatform();
		delete m_platform;
		m_platform = nullptr;
	}


	// ************************************************************************************
	void Engine::throwException(const std::string& msg) {
		m_isolate->ThrowException(newString(msg));
	}

	// ************************************************************************************
	void Engine::getCurrentSourcePaths(std::vector<std::string>& arr) {
		ScriptingScope scope(this);
		v8::Local<v8::StackTrace> trace = v8::StackTrace::CurrentStackTrace(m_isolate, 10, (v8::StackTrace::StackTraceOptions)(v8::StackTrace::kFunctionName | v8::StackTrace::kScriptName));

		for(int i=0;i<trace->GetFrameCount();++i) {
			v8::Local<v8::StackFrame> frame = trace->GetFrame(i);
			std::string loc = converters::ConverterHelper<std::string>::from(this, frame->GetScriptName());
			if (!loc.empty()) {
				arr.push_back(loc);
			}
		}
	}


	// ************************************************************************************
	v8::Local<v8::String> Engine::newString(const std::string& v) {
		return v8::String::NewFromUtf8(m_isolate, v.c_str(), v8::NewStringType::kNormal).ToLocalChecked();
	}

	// ************************************************************************************
	v8::Local<v8::Integer> Engine::newInt(int32_t v) {
		return v8::Int32::New(m_isolate, v);
	}

	// ************************************************************************************
	v8::Local<v8::Number> Engine::newFloat(float v) {
		return v8::Number::New(m_isolate, v);
	}

	// ************************************************************************************
	v8::Local<v8::Number> Engine::newDouble(double v) {
		return v8::Number::New(m_isolate, v);
	}

	// ************************************************************************************
	v8::Local<v8::Boolean> Engine::newBoolean(bool v) {
		return v8::Boolean::New(m_isolate, v);
	}

	// ************************************************************************************
	v8::Local<v8::Value> Engine::newNull() {
		return v8::Null(m_isolate);
	}

	// ************************************************************************************
	v8::Local<v8::Value> Engine::newUndefined() {
		return v8::Undefined(m_isolate);
	}

	// ************************************************************************************
	v8::Local<v8::Array> Engine::newArray(int n) {
		v8::Local<v8::Array> arr = v8::Array::New(m_isolate, n);
		return arr;
	}

	// ************************************************************************************
	v8::Local<v8::External> Engine::newExternal(void* p) {
		return v8::External::New(m_isolate, p);
	}

	// ************************************************************************************
	v8::Local<v8::Value> Engine::parseJSON(const std::string& str) {
		if (str.empty()) return newNull();

		v8::TryCatch tryCatch(m_isolate);
		auto res = v8::JSON::Parse(m_isolate, newString(str));
		if (tryCatch.HasCaught()) return newNull();

		if (res.IsEmpty()) {
			return newNull();
		} else {
			return res.ToLocalChecked();
		}
	}

	// ************************************************************************************
	std::string Engine::toJSONString(v8::Local<v8::Value> val) {
		v8::Local<v8::Object> global = m_context.Get(m_isolate)->Global();
		v8::Local<v8::Object> json = v8::Local<v8::Object>::Cast(global->Get(newString("JSON")));
		v8::Local<v8::Function> func = v8::Local<v8::Function>::Cast(json->Get(newString("stringify")));

		v8::Local<v8::Value> res = func->Call(json, 1, &val);
		return converters::ConverterHelper<std::string>::from(this, res);
	}

	// ************************************************************************************
	v8::Local<v8::Value> Engine::getWithProto(v8::Local<v8::Object> obj, const std::string& name) {
		if (obj.IsEmpty()) return newUndefined();

		v8::Local<v8::Value> n = newString(name);
		while(true) {
			if (obj->Has(n)) return obj->Get(n);
			v8::Local<v8::Value> protoV = obj->GetPrototype();
			if (!protoV.IsEmpty() && protoV->IsObject()) {
				obj = v8::Local<v8::Object>::Cast(protoV);
			} else {
				break;
			}
		}

		return newUndefined();
	}

	// ************************************************************************************
	v8::Local<v8::Function> Engine::newFunctionInternal(const std::string& name, const functions::ScriptFunctor& functor) {
		auto holder = new functions::ScriptFunctorHolder(this, name, functor);

		auto callCallback = [=](const v8::FunctionCallbackInfo<v8::Value>& args){
			v8::Local<v8::External> data = v8::Local<v8::External>::Cast(args.Data());
			functions::ScriptFunctorHolder* holder = (functions::ScriptFunctorHolder*)data->Value();
			//utils::logDebug(stdext::format("Calling native function %s", holder->name));
			holder->call(args);
		};

		auto freeCallback = [=](const v8::WeakCallbackInfo<void>& data){
			functions::ScriptFunctorHolder* holder = (functions::ScriptFunctorHolder*)data.GetParameter();
			//utils::logDebug(stdext::format("Releasing native function %s", holder->name));
			holder->funcPersistent.Reset();
			data.GetIsolate()->AdjustAmountOfExternalAllocatedMemory(-FUNCTION_OBJECT_SIZE);
			delete holder;
		};

		v8::Local<v8::Function> func = v8::Function::New(m_isolate, callCallback, newExternal(holder));
		func->SetName(newString(name));

		holder->funcPersistent.Reset(m_isolate, func);
		holder->funcPersistent.SetWeak((void*)holder, freeCallback, v8::WeakCallbackType::kParameter);
		m_isolate->AdjustAmountOfExternalAllocatedMemory(FUNCTION_OBJECT_SIZE);

		return func;
	}

	// ************************************************************************************
	void Engine::registerSingleton(const std::string& singletonName) {
		ScriptingScope scope(this);

		v8::Local<v8::Object> global = m_context.Get(m_isolate)->Global();
		if (global->HasOwnProperty(newString(singletonName))) {
			utils::logWarning(stdext::format("Singleton %s already registered", singletonName));
			return;
		}

		v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(m_isolate);
		tpl->SetClassName(newString(stdext::format("Singleton_%s", singletonName)));

		v8::Local<v8::Object> obj = tpl->GetFunction()->NewInstance();
		global->Set(newString(singletonName), obj);
	}

	// ************************************************************************************
	void Engine::registerPrototype(const std::string& prototypeName, const std::string& basePrototypeName, const stdext::demangled_name& nativeClassName, v8::FunctionCallback ctor) {
		auto currProto = findPrototypeByName(prototypeName);
		auto baseProto = findPrototypeByName(basePrototypeName);

		if (currProto != nullptr) {
			throw ScriptingException(stdext::format("Prototype %s already registered", prototypeName));
		}

		if (baseProto == nullptr && !basePrototypeName.empty()) {
			throw ScriptingException(stdext::format("Could not find base prototype %s", basePrototypeName));
		}

		ScriptingScope scope(this);
		v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(m_isolate);
		if (baseProto != nullptr) tpl->Inherit(baseProto->GetTemplate());

		if (nativeClassName.empty()) {
			tpl->SetClassName(newString(prototypeName));
		} else {
			tpl->SetClassName(newString(nativeClassName.full()));
		}
		tpl->PrototypeTemplate()->Set(newString("__className"),newString(nativeClassName.full()));
		tpl->InstanceTemplate()->SetInternalFieldCount(4);

		if (ctor != nullptr) {
			tpl->SetCallHandler(ctor, newExternal(this));
		}

		currProto = new Prototype(this);
		currProto->ctor = ctor;
		currProto->nativeClassName = nativeClassName;
		currProto->prototypeName = prototypeName;
		currProto->basePrototype = baseProto;
		currProto->ctor = ctor;
		currProto->tpl.Reset(m_isolate, tpl);

		m_prototypes.push_back(currProto);
		v8::Local<v8::Object> global = m_context.Get(m_isolate)->Global();
		v8::Local<v8::Function> func = tpl->GetFunction();
		func->Set(newString("__protoName"), newString(prototypeName));
		internal::SetObjectPropChain(this, global, prototypeName, func);

		//utils::logDebug(stdext::format("[Engine::registerPrototype] name=%s base=%s native=%s", prototypeName, basePrototypeName, nativeClassName.full()));
	}

	// ************************************************************************************
	void Engine::extendPrototype(const std::string& prototypeName, const std::string& basePrototypeName) {
		auto ctor = [=](const v8::FunctionCallbackInfo<v8::Value>& args){
			Engine* engine = (Engine*)v8::Local<v8::External>::Cast(args.Data())->Value();
			std::string prototypeName = converters::ConverterHelper<std::string>::from(engine, args.Callee()->Get(engine->newString("__protoName")));

			auto proto = engine->findFirstNativePrototypeByName(prototypeName);
			if (proto == nullptr) {
				engine->throwException(stdext::format("Could not find first native prototype for %s", prototypeName));
				return;
			}

			utils::logDebug(stdext::format("new %s() -> calling ctor from proto %s (native %s)", prototypeName, proto->prototypeName, proto->nativeClassName.full()));
			proto->ctor(args);
		};

		registerPrototype(prototypeName, basePrototypeName, stdext::demangled_name(), ctor);
	}

	// ************************************************************************************
	v8::Local<v8::Value> Engine::getGlobalValue(const std::string& name) {
		v8::Local<v8::Object> global = m_context.Get(m_isolate)->Global();
		if (!global.IsEmpty()) {
			v8::Local<v8::Value> key = newString(name);
			return global->Get(key);
		}
		return newUndefined();
	}

	// ************************************************************************************
	v8::Local<v8::Object> Engine::getGlobalObject(const std::string& name) {
		v8::Local<v8::Value> val = getGlobalValue(name);
		if (!val.IsEmpty() && val->IsObject()) {
			return v8::Local<v8::Object>::Cast(val);
		} else {
			return v8::Local<v8::Object>();
		}
	}

	// ************************************************************************************
	v8::Local<v8::Object> Engine::getGlobalObject() {
		v8::Local<v8::Object> global = m_context.Get(m_isolate)->Global();
		return global;
	}

	// ************************************************************************************
	v8::Local<v8::Function> Engine::getGlobalFunction(const std::string& name) {
		v8::Local<v8::Value> val = getGlobalValue(name);
		if (!val.IsEmpty() && val->IsFunction()) {
			return v8::Local<v8::Function>::Cast(val);
		} else {
			return v8::Local<v8::Function>();
		}
	}

	// ************************************************************************************
	Engine::Prototype* Engine::findPrototypeByName(const std::string& name) {
		for(auto& p: m_prototypes) {
			if (p->prototypeName == name) return p;
		}
		return nullptr;
	}

	// ************************************************************************************
	Engine::Prototype* Engine::findPrototypeByNativeClassName(const stdext::demangled_name& name) {
		for(auto& p: m_prototypes) {
			if (p->nativeClassName == name) return p;
		}
		return nullptr;
	}

	// ************************************************************************************
	Engine::Prototype* Engine::findFirstNativePrototypeByName(const std::string& name) {
		Prototype* proto = findPrototypeByName(name);
		while(proto != nullptr) {
			if (!proto->nativeClassName.empty()) break;
			proto = proto->basePrototype;
		}
		return proto;
	}

	// ************************************************************************************
	void Engine::runString(const std::string& s_origin, const std::string& s_content) {
		ScriptingScope scope(this);

		v8::ScriptOrigin origin(newString(s_origin));
		v8::Local<v8::Script> script;
		v8::Local<v8::String> content = newString(s_content);

		if (v8::Script::Compile(scope.context(), content, &origin).ToLocal(&script)) {
			v8::Local<v8::Value> result;
			script->Run(scope.context());
		}
		scope.checkThrowException();
	}

	// ************************************************************************************
	void Engine::runFile(const std::string& path) {
		std::string content = utils::readFileContents(path);
		runString(path, content);
	}

	// ************************************************************************************
	v8::Local<v8::Function> Engine::compileFunctionRaw(const std::string& sOrigin, const std::string& func) {
		v8::ScriptOrigin origin(newString(sOrigin));
		v8::Local<v8::Script> script;
		v8::Local<v8::String> content = newString(stdext::format("(%s)",func));

		if (v8::Script::Compile(context(), content, &origin).ToLocal(&script)) {
			v8::Local<v8::Value> result;
			if (script->Run(context()).ToLocal(&result)) {
				if (!result.IsEmpty() && result->IsFunction()) {
					return v8::Local<v8::Function>::Cast(result);
				} else {
					return v8::Local<v8::Function>();
				}
			}

		}
		return v8::Local<v8::Function>();
	}

	// ************************************************************************************
	void Engine::CallInObjectContext(v8::Local<v8::Object> obj, const std::string& sOrigin, const std::string& sCode) {
		ScriptingScope scope(this);

		v8::ScriptOrigin origin(newString(sOrigin));
		v8::Local<v8::Script> script;
		v8::Local<v8::String> content = newString(stdext::format("(function(){ %s })",sCode));

		if (v8::Script::Compile(scope.context(), content, &origin).ToLocal(&script)) {
			v8::Local<v8::Value> result;
			if (script->Run(scope.context()).ToLocal(&result)) {
				if (!result.IsEmpty() && result->IsFunction()) {
					v8::Local<v8::Function> func = v8::Local<v8::Function>::Cast(result);
					func->CallAsFunction(scope.context(), obj, 0, nullptr);
				}
			}
		}
		scope.checkThrowException();
	}

	// ************************************************************************************
	void Engine::gc() {
		v8::HeapStatistics stats;
		m_isolate->GetHeapStatistics(&stats);
		std::size_t used1 = stats.used_heap_size();

		while (!m_isolate->IdleNotification(1000)) ;

		m_isolate->GetHeapStatistics(&stats);
		std::size_t used2 = stats.used_heap_size();

		utils::logDebug(stdext::format("Scripting GC. Freed %lu bytes. prototypes=%d", (used1 - used2), m_prototypes.size()));
	}


}


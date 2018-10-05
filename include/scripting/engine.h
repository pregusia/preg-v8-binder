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


#ifndef INCLUDE_SCRIPTING_ENGINE_H_
#define INCLUDE_SCRIPTING_ENGINE_H_

#include "base.h"
#include <v8.h>
#include <functional>

namespace scripting {

	namespace internal {
		class ObjectWrapperData;
	}
	namespace functions {
		typedef std::function<void(Engine* engine, const std::string& funcName, const v8::FunctionCallbackInfo<v8::Value>)> ScriptFunctor;
	}

	class Engine;
	class ScriptableObject;
	class ScriptingScope;

	class ScriptingException : public std::exception {
		public:
			ScriptingException(const std::string& msg) : m_msg(msg) { }
			virtual ~ScriptingException() throw() { }

			virtual const char* what() const throw() { return m_msg.c_str(); }
		private:
			std::string m_msg;
	};


	class Engine {
		public:
			bool m_suppressCtorCallback;

			static const std::size_t FUNCTION_OBJECT_SIZE = 1024;
			static const char* CORE_SCRIPT;

			Engine();
			~Engine();

			// helpers method
			void throwException(const std::string& msg);
			void getCurrentSourcePaths(std::vector<std::string>& arr);

			v8::Local<v8::Value> newNull();
			v8::Local<v8::Value> newUndefined();
			v8::Local<v8::String> newString(const std::string& v);
			v8::Local<v8::Integer> newInt(int32_t v);
			v8::Local<v8::Number> newFloat(float v);
			v8::Local<v8::Number> newDouble(double v);
			v8::Local<v8::Boolean> newBoolean(bool v);
			v8::Local<v8::Array> newArray(int n);
			v8::Local<v8::External> newExternal(void* ptr);

			v8::Local<v8::Value> getWithProto(v8::Local<v8::Object> obj, const std::string& name);

			template<typename... Args>
			v8::Local<v8::Array> newArrayValues(const Args&... args);

			template<typename... Args>
			v8::Local<v8::Object> newAnonymousObject(const Args&... args);

			template<typename... Args>
			v8::Local<v8::Object> newObjectFromPrototype(const std::string& prototypeName, const Args&... args);

			template<typename F>
			v8::Local<v8::Function> newFunction(const std::string& name, const F& func);

			v8::Local<v8::Value> parseJSON(const std::string& str);
			std::string toJSONString(v8::Local<v8::Value> val);

			// global
			template<typename F>
			void registerGlobalStaticFunction(const std::string& name, const F& func);

			// prototypes
			void registerPrototype(const std::string& prototypeName, const std::string& basePrototypeName, const stdext::demangled_name& nativeClassName, v8::FunctionCallback ctor);
			void extendPrototype(const std::string& prototypeName, const std::string& basePrototypeName);

			// classes releated
			template<class C, class B = ScriptableObject>
			void registerNativeClass(const std::string& ns);

			template<typename CLS, typename F>
			void registerNativeClassMemberFunction(const std::string& name, const F& func);

			template<typename CLS, typename F>
			void registerNativeClassStaticFunction(const std::string& name, const F& func);

			template<typename CLS, typename F>
			void registerNativeClassFactoryFunction(const std::string& name, const F& func);

			template<typename CLS, typename GETTER>
			void registerNativeClassPropertyAccessor(const std::string& name, const GETTER& getter);

			template<typename CLS, typename GETTER, typename SETTER>
			void registerNativeClassPropertyAccessor(const std::string& name, const GETTER& getter, const SETTER& setter);


			// singleton
			void registerSingleton(const std::string& singletonName);

			template<typename CLS, typename F>
			void registerSingletonMemberFunction(const std::string& singletonName, const std::string& methodName, const F& func, CLS* inst);

			template<typename F>
			void registerSingletonStaticFunction(const std::string& singletonName, const std::string& methodName, const F& func);

			template<typename GETTER>
			void registerSingletonPropertyAccessor(const std::string& singletonName, const std::string& propertyName, const GETTER& getter);

			template<typename GETTER, typename SETTER>
			void registerSingletonPropertyAccessor(const std::string& singletonName, const std::string& propertyName, const GETTER& getter, const SETTER& setter);


			// object manipulation
			template<typename R, typename... Args>
			R CallScriptFunction(v8::Local<v8::Value> f, v8::Local<v8::Object> self, Args... args);

			template<typename R, typename... Args>
			R CallObjectPropertyWithSelf(v8::Local<v8::Object> obj, v8::Local<v8::Object> self, const std::string& propName, const Args... args);

			template<typename R, typename... Args>
			R CallObjectProperty(v8::Local<v8::Object> obj, const std::string& propName, const Args... args);

			template<typename T>
			T GetObjectProperty(v8::Local<v8::Object> obj, const std::string& propName);

			template<typename... Args>
			bool CallGlobalObjectEvent(const std::string& objName, const std::string& name, Args... args);

			// other methods

			void runFile(const std::string& path);
			void runString(const std::string& origin, const std::string& content);
			void gc();

			template<typename RET, typename... Args>
			std::function<RET(Args...)> compileFunction(const std::string& origin, const std::string& func);

			v8::Local<v8::Function> compileFunctionRaw(const std::string& origin, const std::string& func);

			void CallInObjectContext(v8::Local<v8::Object> obj, const std::string& origin, const std::string& code);

			v8::Isolate* isolate() { return m_isolate; }
			v8::Local<v8::Context> context() { return m_context.Get(m_isolate); }
			v8::Local<v8::Value> getGlobalValue(const std::string& name);
			v8::Local<v8::Object> getGlobalObject(const std::string& name);
			v8::Local<v8::Object> getGlobalObject();
			v8::Local<v8::Function> getGlobalFunction(const std::string& name);

			// prototypes
			class Prototype {
				public:
					Engine* engine;
					v8::Persistent<v8::FunctionTemplate> tpl;
					Prototype* basePrototype;
					std::string prototypeName;
					stdext::demangled_name nativeClassName;
					v8::FunctionCallback ctor;

					Prototype(Engine* engine) : engine(engine), basePrototype(nullptr), ctor(nullptr) { }
					v8::Local<v8::FunctionTemplate> GetTemplate() { return tpl.Get(engine->isolate()); }
					v8::Local<v8::Object> NewInstance() {
						engine->m_suppressCtorCallback = true;
						v8::Local<v8::Object> v = GetTemplate()->GetFunction()->NewInstance();
						engine->m_suppressCtorCallback = false;
						return v;
					}
			};

			Prototype* findPrototypeByName(const std::string& name);
			Prototype* findPrototypeByNativeClassName(const stdext::demangled_name& name);
			Prototype* findFirstNativePrototypeByName(const std::string& name);

		private:
			v8::Persistent<v8::Context> m_context;
			v8::Isolate* m_isolate;
			v8::Platform* m_platform;

			std::vector<Prototype*> m_prototypes;

			v8::Local<v8::Function> newFunctionInternal(const std::string& name, const functions::ScriptFunctor& func);

			friend class ScriptingScope;
	};

	class ScriptingScope {
		private:
			ScriptingScope(const ScriptingScope& from);
			ScriptingScope& operator=(const ScriptingScope& a);

			Engine* m_engine;
			v8::Locker m_locker;
			v8::Isolate::Scope m_isolateScope;
			v8::HandleScope m_handleScope;
			v8::Context::Scope m_contextScope;
			v8::TryCatch m_tryCatch;


		public:
			ScriptingScope(Engine* e) : m_engine(e), m_locker(e->m_isolate), m_isolateScope(e->m_isolate), m_handleScope(e->m_isolate), m_contextScope(e->m_context.Get(e->m_isolate)), m_tryCatch(e->m_isolate) { }
			v8::Local<v8::Context> context() { return m_engine->m_context.Get(m_engine->m_isolate); }

			v8::Local<v8::String> newString(const std::string& v) { return m_engine->newString(v); }
			v8::Local<v8::Number> newInt(int32_t v) { return m_engine->newInt(v); }
			v8::Local<v8::Value> newNull() { return m_engine->newNull(); }


			void checkThrowException(bool clear=true);
			bool wasException(bool clear=true);

	};

}

#define INCLUDING_FROM_ENGINE
#	include "converters.h"
#	include "object.h"
#	include "internal.h"
#	include "functionwrapper.h"
#undef INCLUDING_FROM_ENGINE

// converters for std::function
namespace scripting { namespace converters {

	template<typename T>
	v8::Local<v8::Value> convertTo(Engine* engine, const stdext::object_ptr<T>& v) {
		static_assert(std::is_base_of<ScriptableObject, T>::value, "cannot convert not base of ScriptableObject");

		if (v.empty()) return engine->newNull();

		ScriptableObject* obj = dynamic_cast<ScriptableObject*>(v.get());
		if (obj == nullptr) return engine->newNull();

		return obj->scriptingGetObject();
	}

	template<typename T>
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, stdext::object_ptr<T>& out) {
		static_assert(std::is_base_of<ScriptableObject, T>::value, "cannot convert not base of ScriptableObject");

		out.reset();
		if (v.IsEmpty()) return;
		if (!v->IsObject()) return;

		v8::Local<v8::Object> obj = v8::Local<v8::Object>::Cast(v);
		if (obj->InternalFieldCount() == 0) return;

		auto scriptObject = (stdext::object_ptr<ScriptableObject>*)v8::Local<v8::External>::Cast(obj->GetInternalField(0))->Value();
		if (scriptObject == nullptr) return;

		out = (*scriptObject)->dynamic_self_cast<T>();
	}

	template<typename T>
	v8::Local<v8::Value> convertTo(Engine* engine, const std::vector<T>& v) {
		v8::Local<v8::Array> arr = engine->newArray(v.size());
		for(std::size_t i=0;i<v.size();++i) {
			arr->Set(i, convertTo(engine, v[i]));
		}
		return arr;
	}

	template<typename T>
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, std::vector<T>& out) {
		if (v.IsEmpty()) return;
		if (!v->IsArray()) return;
		v8::Local<v8::Array> arr = v8::Local<v8::Array>::Cast(v);

		out.clear();
		for(uint32_t i=0;i<arr->Length();++i) {
			out.push_back(ConverterHelper<T>::from(engine, arr->Get(i)));
		}
	}

	template<typename T>
	v8::Local<v8::Value> convertTo(Engine* engine, const std::list<T>& v) {
		v8::Local<v8::Array> arr = engine->newArray(v.size());
		int32_t i=0;
		for(auto it=v.begin();it != v.end();++it) {
			arr->Set(i, convertTo(engine, *it));
			i += 1;
		}
		return arr;
	}

	template<typename T>
	v8::Local<v8::Value> convertTo(Engine* engine, const std::deque<T>& v) {
		v8::Local<v8::Array> arr = engine->newArray(v.size());
		int32_t i=0;
		for(auto it=v.begin();it != v.end();++it) {
			arr->Set(i, convertTo(engine, *it));
			i += 1;
		}
		return arr;
	}

	template<typename T>
	v8::Local<v8::Value> convertTo(Engine* engine, const std::map<int, T>& v) {
		v8::Local<v8::Object> obj = engine->newAnonymousObject();
		for(auto it=v.begin();it != v.end();++it) {
			obj->CreateDataProperty(engine->context(), it->first, convertTo(engine, it->second));
		}
		return obj;
	}

	template<typename T>
	v8::Local<v8::Value> convertTo(Engine* engine, const std::map<std::string, T>& v) {
		v8::Local<v8::Object> obj = engine->newAnonymousObject();
		for(auto it=v.begin();it != v.end();++it) {
			obj->CreateDataProperty(engine->context(), engine->newString(it->first), convertTo(engine, it->second));
		}
		return obj;
	}

	template<typename R, typename... Args>
	v8::Local<v8::Value> convertTo(Engine* engine, const std::function<R(Args...)>& f) {
		return engine->newFunction("<anonymous>",f);
	}

	template<typename R, typename... Args>
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, std::function<R(Args...)>& out) {
		functions::ScriptFunctionCallerPtr caller(new functions::ScriptFunctionCaller(engine, v));
		out = functions::ScriptFunctionCallerExecutor<R,Args...>(caller);
	}

	template<typename T,class=typename std::enable_if<std::is_enum<T>::value>::type>
	v8::Local<v8::Value> convertTo(Engine* engine, const T& v) {
		return engine->newInt((int32_t)v);
	}

	template<typename T,class=typename std::enable_if<std::is_enum<T>::value>::type>
	void convertFrom(Engine* engine, v8::Local<v8::Value> v, T& out) {
		int32_t tmp = 0;
		convertFrom(engine, v, tmp);
		out = (T)tmp;
	}

} }


namespace scripting {

	// ************************************************************************************
	template<typename... Args>
	v8::Local<v8::Array> Engine::newArrayValues(const Args&... args) {
		int n = sizeof...(Args);
		v8::Local<v8::Array> arr = v8::Array::New(m_isolate, n);
		internal::SetArrayValues(this, arr, args...);
		return arr;
	}

	// ************************************************************************************
	template<typename... Args>
	v8::Local<v8::Object> Engine::newAnonymousObject(const Args&... args) {
		v8::Local<v8::Object> obj = v8::Object::New(m_isolate);
		internal::SetObjectProps(this, obj, args...);
		return obj;
	}

	// ************************************************************************************
	template<typename... Args>
	v8::Local<v8::Object> Engine::newObjectFromPrototype(const std::string& prototypeName, const Args&... args) {
		Prototype* proto = findPrototypeByName(prototypeName);
		if (proto == nullptr) throw ScriptingException(stdext::format("Could not find prototype %s", prototypeName));

		v8::Local<v8::Object> obj = proto->NewInstance();
		internal::SetObjectProps(this, obj, args...);
		return obj;
	}

	// ************************************************************************************
	template<typename F>
	v8::Local<v8::Function> Engine::newFunction(const std::string& name, const F& func) {
		return newFunctionInternal(name, functions::makeStatic(func));
	}

	// ************************************************************************************
	template<typename F>
	void Engine::registerGlobalStaticFunction(const std::string& funcName, const F& func) {
		ScriptingScope scope(this);

		// trzeba utworzyc funkcje
		v8::Local<v8::Function> tpl = newFunctionInternal(
			funcName,
			functions::makeStatic(func)
		);

		v8::Local<v8::Object> global = m_context.Get(m_isolate)->Global();
		auto ff = compileFunctionRaw(
			stdext::format("<registerGlobalMethod %s>", funcName),
			stdext::format("function(a0){ this.%s = a0; }", funcName)
		);
		assert(!ff.IsEmpty());

		v8::Local<v8::Value> args[3];
		args[0] = tpl;
		ff->Call(context(), global, 1, args);

		scope.checkThrowException();
	}

	// ************************************************************************************
	template<class C, class B>
	void Engine::registerNativeClass(const std::string& ns) {
		auto baseNativeClassName = stdext::demangled_name::get<B>();
		auto currNativeClassName = stdext::demangled_name::get<C>();

		Prototype* basePrototype = findPrototypeByNativeClassName(baseNativeClassName);
		if (basePrototype == nullptr) throw ScriptingException(stdext::format("Could not find prototype for native class %s", baseNativeClassName.full()));

		std::string prototypeName = internal::normalizePrototypeName(currNativeClassName.last(), ns);
		registerPrototype(prototypeName, basePrototype->prototypeName, currNativeClassName, &functions::ConstructorCallback<C>);
	}

	// ************************************************************************************
	template<typename CLS, typename F>
	void Engine::registerNativeClassMemberFunction(const std::string& methodName, const F& func) {
		ScriptingScope scope(this);
		auto nativeClassName = stdext::demangled_name::get<CLS>();
		auto prototype = findPrototypeByNativeClassName(nativeClassName);
		if (prototype == nullptr) throw ScriptingException(stdext::format("Could not find prototype for %s", nativeClassName.full()));

		// trzeba utworzyc funkcje
		v8::Local<v8::Function> tpl = newFunctionInternal(
			stdext::format("[%s].%s", nativeClassName.full(), methodName),
			functions::makeClassMember<CLS>(func)
		);

		v8::Local<v8::Object> global = m_context.Get(m_isolate)->Global();
		auto ff = compileFunctionRaw(
			stdext::format("<registerClassMethod [%s].%s>", nativeClassName.full(),methodName),
			stdext::format("function(a0){ Object.defineProperty(%s.prototype, '%s', { writable: false, enumerable: true, configurable: false, value: a0 }); }", prototype->prototypeName, methodName)
		);
		assert(!ff.IsEmpty());

		v8::Local<v8::Value> args[3];
		args[0] = tpl;
		ff->Call(context(), global, 1, args);

		scope.checkThrowException();
	}

	// ************************************************************************************
	template<typename CLS, typename F>
	void Engine::registerNativeClassStaticFunction(const std::string& methodName, const F& func) {
		ScriptingScope scope(this);
		auto nativeClassName = stdext::demangled_name::get<CLS>();
		auto prototype = findPrototypeByNativeClassName(nativeClassName);
		if (prototype == nullptr) throw ScriptingException(stdext::format("Could not find prototype for %s", nativeClassName.full()));

		// trzeba utworzyc funkcje
		v8::Local<v8::Function> tpl = newFunctionInternal(
			stdext::format("[%s].%s", nativeClassName.full(), methodName),
			functions::makeStatic(func)
		);

		v8::Local<v8::Object> global = m_context.Get(m_isolate)->Global();
		auto ff = compileFunctionRaw(
			stdext::format("<registerClassMethod [%s].%s>", nativeClassName.full(),methodName),
			stdext::format("function(a0){ Object.defineProperty(%s.prototype, '%s', { writable: false, enumerable: true, configurable: false, value: a0 }); }", prototype->prototypeName, methodName)
		);
		assert(!ff.IsEmpty());

		v8::Local<v8::Value> args[3];
		args[0] = tpl;
		ff->Call(context(), global, 1, args);

		scope.checkThrowException();
	}

	// ************************************************************************************
	template<typename CLS, typename F>
	void Engine::registerNativeClassFactoryFunction(const std::string& methodName, const F& func) {
		ScriptingScope scope(this);
		auto nativeClassName = stdext::demangled_name::get<CLS>();
		auto prototype = findPrototypeByNativeClassName(nativeClassName);
		if (prototype == nullptr) throw ScriptingException(stdext::format("Could not find prototype for %s", nativeClassName.full()));

		// trzeba utworzyc funkcje
		v8::Local<v8::Function> f = newFunctionInternal(
			stdext::format("[%s].%s", nativeClassName.full(), methodName),
			functions::makeStatic(func)
		);

		v8::Local<v8::Object> global = m_context.Get(m_isolate)->Global();
		auto ff = compileFunctionRaw(
			stdext::format("<registerClassMethod [%s].%s>", nativeClassName.full(),methodName),
			stdext::format("function(a0){ Object.defineProperty(%s, '%s', { writable: false, enumerable: true, configurable: false, value: a0 }); }", prototype->prototypeName, methodName)
		);
		assert(!ff.IsEmpty());

		v8::Local<v8::Value> args[3];
		args[0] = f;
		ff->Call(context(), global, 1, args);

		scope.checkThrowException();
	}

	// ************************************************************************************
	template<typename CLS, typename GETTER>
	void Engine::registerNativeClassPropertyAccessor(const std::string& propName, const GETTER& getter) {
		ScriptingScope scope(this);
		auto nativeClassName = stdext::demangled_name::get<CLS>();
		auto prototype = findPrototypeByNativeClassName(nativeClassName);
		if (prototype == nullptr) throw ScriptingException(stdext::format("Could not find prototype for %s", nativeClassName.full()));

		// trzeba utworzyc funkcje
		v8::Local<v8::Function> getterFunc = newFunctionInternal(
			stdext::format("[%s].[getter:%s]", nativeClassName.full(), propName),
			functions::makeClassMember<CLS>(getter)
		);

		// ustawianie
		v8::Local<v8::Object> global = m_context.Get(m_isolate)->Global();
		auto func = compileFunctionRaw(
			stdext::format("<registerClassProperty [%s].%s>", nativeClassName.full(),propName),
			stdext::format("function(getter,setter){ Object.defineProperty(%s.prototype, '%s', { enumerable: true, configurable: false, get: getter, set: setter }); }", prototype->prototypeName, propName)
		);
		assert(!func.IsEmpty());

		v8::Local<v8::Value> args[3];
		args[0] = getterFunc;
		args[1] = newUndefined();
		func->Call(context(), global, 2, args);

		scope.checkThrowException();
	}

	// ************************************************************************************
	template<typename CLS, typename GETTER, typename SETTER>
	void Engine::registerNativeClassPropertyAccessor(const std::string& propName, const GETTER& getter, const SETTER& setter) {
		ScriptingScope scope(this);
		auto nativeClassName = stdext::demangled_name::get<CLS>();
		auto prototype = findPrototypeByNativeClassName(nativeClassName);
		if (prototype == nullptr) throw ScriptingException(stdext::format("Could not find prototype for %s", nativeClassName.full()));

		// trzeba utworzyc funkcje
		v8::Local<v8::Function> getterFunc = newFunctionInternal(
			stdext::format("[%s].[getter:%s]", nativeClassName.full(), propName),
			functions::makeClassMember<CLS>(getter)
		);
		v8::Local<v8::Function> setterFunc = newFunctionInternal(
			stdext::format("[%s].[setter:%s]", nativeClassName.full(), propName),
			functions::makeClassMember<CLS>(setter)
		);

		// ustawianie
		v8::Local<v8::Object> global = m_context.Get(m_isolate)->Global();
		auto func = compileFunctionRaw(
			stdext::format("<registerClassProperty [%s].%s>", nativeClassName.full(),propName),
			stdext::format("function(getter,setter){ Object.defineProperty(%s.prototype, '%s', { enumerable: true, configurable: false, get: getter, set: setter }); }", prototype->prototypeName, propName)
		);
		assert(!func.IsEmpty());

		v8::Local<v8::Value> args[3];
		args[0] = getterFunc;
		args[1] = setterFunc;
		func->Call(context(), global, 2, args);

		scope.checkThrowException();
	}

	// ************************************************************************************
	template<typename CLS, typename F>
	void Engine::registerSingletonMemberFunction(const std::string& singletonName, const std::string& methodName, const F& func, CLS* inst) {
		ScriptingScope scope(this);

		auto obj = getGlobalObject(singletonName);
		if (obj.IsEmpty()) {
			throw ScriptingException(stdext::format("Singleton %s not found"));
			return;
		}

		v8::Local<v8::Function> tpl = newFunctionInternal(
			methodName,
			functions::makeSingletonMember(inst, func)
		);

		v8::Local<v8::Object> global = m_context.Get(m_isolate)->Global();
		auto ff = compileFunctionRaw(
			stdext::format("<registerSingletonMethod %s.%s>", singletonName,methodName),
			stdext::format("function(a0){ Object.defineProperty(%s, '%s', { writable: false, enumerable: true, configurable: false, value: a0 }); }", singletonName, methodName)
		);
		assert(!ff.IsEmpty());

		v8::Local<v8::Value> args[3];
		args[0] = tpl;
		ff->Call(context(), global, 1, args);

		scope.checkThrowException();
	}

	// ************************************************************************************
	template<typename F>
	void Engine::registerSingletonStaticFunction(const std::string& singletonName, const std::string& methodName, const F& func) {
		ScriptingScope scope(this);

		auto obj = getGlobalObject(singletonName);
		if (obj.IsEmpty()) {
			throw ScriptingException(stdext::format("Singleton %s not found"));
			return;
		}

		v8::Local<v8::Function> tpl = newFunctionInternal(
			methodName,
			functions::makeStatic(func)
		);

		v8::Local<v8::Object> global = m_context.Get(m_isolate)->Global();
		auto ff = compileFunctionRaw(
			stdext::format("<registerSingletonMethod %s.%s>", singletonName,methodName),
			stdext::format("function(a0){ Object.defineProperty(%s, '%s', { writable: false, enumerable: true, configurable: false, value: a0 }); }", singletonName, methodName)
		);
		assert(!ff.IsEmpty());

		v8::Local<v8::Value> args[3];
		args[0] = tpl;
		ff->Call(context(), global, 1, args);

		scope.checkThrowException();
	}

	// ************************************************************************************
	template<typename GETTER>
	void Engine::registerSingletonPropertyAccessor(const std::string& singletonName, const std::string& propName, const GETTER& getter) {
		ScriptingScope scope(this);

		auto obj = getGlobalObject(singletonName);
		if (obj.IsEmpty()) {
			throw ScriptingException(stdext::format("Singleton %s not found"));
			return;
		}

		// trzeba utworzyc funkcje
		v8::Local<v8::Function> getterFunc = newFunctionInternal(
			stdext::format("%s.%s", singletonName, propName),
			functions::makeStatic(getter)
		);

		// ustawianie
		v8::Local<v8::Object> global = m_context.Get(m_isolate)->Global();
		auto func = compileFunctionRaw(
			stdext::format("<registerSingletonPropertyAccessor %s.%s>", singletonName,propName),
			stdext::format("function(getter,setter){ Object.defineProperty(%s, '%s', { enumerable: true, configurable: false, get: getter, set: setter }); }", singletonName, propName)
		);
		assert(!func.IsEmpty());

		v8::Local<v8::Value> args[3];
		args[0] = getterFunc;
		args[1] = newUndefined();
		func->Call(context(), global, 2, args);

		scope.checkThrowException();
	}

	// ************************************************************************************
	template<typename GETTER, typename SETTER>
	void Engine::registerSingletonPropertyAccessor(const std::string& singletonName, const std::string& propName, const GETTER& getter, const SETTER& setter) {
		ScriptingScope scope(this);

		auto obj = getGlobalObject(singletonName);
		if (obj.IsEmpty()) {
			throw ScriptingException(stdext::format("Singleton %s not found"));
			return;
		}

		// trzeba utworzyc funkcje
		v8::Local<v8::Function> getterFunc = newFunctionInternal(
			stdext::format("%s.%s", singletonName, propName),
			functions::makeStatic(getter)
		);
		v8::Local<v8::Function> setterFunc = newFunctionInternal(
			stdext::format("%s.%s", singletonName, propName),
			functions::makeStatic(setter)
		);

		// ustawianie
		v8::Local<v8::Object> global = m_context.Get(m_isolate)->Global();
		auto func = compileFunctionRaw(
			stdext::format("<registerSingletonPropertyAccessor %s.%s>", singletonName,propName),
			stdext::format("function(getter,setter){ Object.defineProperty(%s, '%s', { enumerable: true, configurable: false, get: getter, set: setter }); }", singletonName, propName)
		);
		assert(!func.IsEmpty());

		v8::Local<v8::Value> args[3];
		args[0] = getterFunc;
		args[1] = setterFunc;
		func->Call(context(), global, 2, args);

		scope.checkThrowException();
	}


	// ************************************************************************************
	template<typename R, typename... Args>
	R Engine::CallScriptFunction(v8::Local<v8::Value> f, v8::Local<v8::Object> self, Args... args) {
		if (f.IsEmpty()) return R();
		if (!f->IsFunction()) return R();

		v8::Local<v8::Value> arr[sizeof...(Args)];
		internal::MapArgs(this, arr, args...);

		auto ret = v8::Local<v8::Function>::Cast(f)->CallAsFunction(m_context.Get(m_isolate), self, sizeof...(Args), arr);
		if (!ret.IsEmpty()) {
			return converters::ConverterHelper<R>::from(this, ret.ToLocalChecked());
		} else {
			return R();
		}
	}

	// ************************************************************************************
	template<typename R, typename... Args>
	R Engine::CallObjectPropertyWithSelf(v8::Local<v8::Object> obj, v8::Local<v8::Object> self, const std::string& propName, const Args... args) {
		v8::Local<v8::Value> funcV = getWithProto(obj, propName);
		if (!funcV.IsEmpty() && funcV->IsFunction()) {
			v8::Local<v8::Function> f = v8::Local<v8::Function>::Cast(funcV);
			return CallScriptFunction<R>(f, self, args...);
		}
		utils::logWarning(stdext::format("Could not find function %s", propName));
		return R();
	}

	// ************************************************************************************
	template<typename R, typename... Args>
	R Engine::CallObjectProperty(v8::Local<v8::Object> obj, const std::string& propName, const Args... args) {

		v8::Local<v8::Object> self = obj;
		v8::Local<v8::String> pn = newString(propName);

		while(!obj.IsEmpty()) {
			v8::Local<v8::Value> val = obj->Get(pn);
			if (!val.IsEmpty() && val->IsFunction()) {
				v8::Local<v8::Function> f = v8::Local<v8::Function>::Cast(val);
				return CallScriptFunction<R>(f, self, args...);
			}

			v8::Local<v8::Value> proto = obj->GetPrototype();
			if (!proto.IsEmpty() && proto->IsObject()) {
				obj = v8::Local<v8::Object>::Cast(proto);
			} else {
				break;
			}
		}
		utils::logWarning(stdext::format("Could not find function %s", propName));
		return R();
	}

	// ************************************************************************************
	template<typename T>
	T Engine::GetObjectProperty(v8::Local<v8::Object> obj, const std::string& propName) {
		if (!obj.IsEmpty()) {
			if (obj->IsObject()) {
				v8::Local<v8::Value> val = obj->Get(newString(propName));
				T ret = T();
				converters::convertFrom(this, val, ret);
				return ret;
			}
		}
		return T();
	}

	// ************************************************************************************
	template<typename... Args>
	bool Engine::CallGlobalObjectEvent(const std::string& objName, const std::string& name, Args... args) {
		ScriptingScope scope(this);

		v8::Local<v8::Object> obj = getGlobalObject(objName);
		v8::Local<v8::Function> dollarFunc = getGlobalFunction("$");

		if (!dollarFunc.IsEmpty() && !obj.IsEmpty()) {
			bool res = g_engineScripting->CallObjectProperty<bool>(dollarFunc, "eventCallStatic", obj, name, args...);
			scope.checkThrowException();
			return res;
		}

		return false;
	}


	// ************************************************************************************
	template<typename RET, typename... Args>
	std::function<RET(Args...)> Engine::compileFunction(const std::string& sOrigin, const std::string& func) {
		ScriptingScope scope(this);

		v8::ScriptOrigin origin(newString(sOrigin));
		v8::Local<v8::Script> script;
		v8::Local<v8::String> content = newString(stdext::format("(%s)",func));

		if (v8::Script::Compile(scope.context(), content, &origin).ToLocal(&script)) {
			v8::Local<v8::Value> result;
			if (script->Run(scope.context()).ToLocal(&result)) {
				scope.checkThrowException();

				if (!result.IsEmpty() && result->IsFunction()) {
					functions::ScriptFunctionCallerPtr caller(new functions::ScriptFunctionCaller(this, result));
					return functions::ScriptFunctionCallerExecutor<RET,Args...>(caller);
				} else {
					return nullptr;
				}
			}

		}
		scope.checkThrowException();
		return nullptr;
	}

}

#endif /* INCLUDE_SCRIPTING_ENGINE_H_ */

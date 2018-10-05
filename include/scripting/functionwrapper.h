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


#ifndef INCLUDING_FROM_ENGINE
#	error "This file should only be included internally"
#endif

#ifndef INCLUDE_SCRIPTING_FUNCTIONWRAPPER_H_
#define INCLUDE_SCRIPTING_FUNCTIONWRAPPER_H_

#include <v8.h>
#include <functional>

namespace scripting {
	class Engine;
}

namespace scripting { namespace functions {

	typedef std::function<void(Engine* engine, const std::string& funcName, const v8::FunctionCallbackInfo<v8::Value>)> ScriptFunctor;



	// ******************************************************************************************************************************
	// static
	// ******************************************************************************************************************************

	namespace impl {
		template<typename F, typename RET, typename... Args>
		ScriptFunctor makeStaticInternal(const F& func, RET(F::*method)(Args...) const) {
			return [=](Engine* engine, const std::string& funcName, const v8::FunctionCallbackInfo<v8::Value>& args) {
				if (args.IsConstructCall()) {
					engine->throwException(stdext::format("Could not call function %s in ctor context", funcName));
					return;
				}

				std::tuple<typename stdext::remove_const_ref<Args>::type...> argsTuple = internal::UnmapArgs<typename stdext::remove_const_ref<Args>::type...>(engine, args);
				args.GetReturnValue().Set(internal::CallClassFunctionFromTupleMapReturn<RET>(engine, &func, method, argsTuple));
			};
		}
	}

	template<typename F>
	ScriptFunctor makeStatic(const F& func) {
		//typedef decltype(&F::operator()) M;
		return impl::makeStaticInternal<F>(func, &F::operator());
	};

	template<typename RET, typename... Args>
	ScriptFunctor makeStatic(RET(*func)(Args...)) {
		typedef std::function<RET(Args...)> FuncType;
		auto f = std::function<RET(Args...)>(func);
		return makeStatic<FuncType>(f);
	}

	// ******************************************************************************************************************************
	// class.member
	// ******************************************************************************************************************************

	namespace impl {
		template<typename CLS, typename F, typename RET, typename... Args>
		ScriptFunctor makeClassMemberInternal(const F& func, RET(F::*method)(CLS*, Args...) const) {
			return [=](Engine* engine, const std::string& funcName, const v8::FunctionCallbackInfo<v8::Value>& args) {
				if (args.IsConstructCall()) {
					engine->throwException(stdext::format("Could not call function %s in ctor context", funcName));
					return;
				}

				CLS* instance = ScriptableObject::unwrap<CLS>(args.This());
				if (instance == nullptr) {
					engine->throwException(stdext::format("Could not unwrap object for calling %s", funcName));
					return;
				}

				std::tuple<typename stdext::remove_const_ref<Args>::type...> tmpTuple = internal::UnmapArgs<typename stdext::remove_const_ref<Args>::type...>(engine, args);
				std::tuple<CLS*, typename stdext::remove_const_ref<Args>::type...> argsTuple = std::tuple_cat(std::make_tuple(instance), tmpTuple);

				args.GetReturnValue().Set(internal::CallClassFunctionFromTupleMapReturn<RET>(engine, &func, method, argsTuple));
			};
		}
	}

	template<typename CLS, typename F>
	ScriptFunctor makeClassMember(const F& func) {
		//typedef decltype(&F::operator()) M;
		return impl::makeClassMemberInternal<CLS,F>(func, &F::operator());
	};

	template<typename CLS, typename RET, typename... Args>
	ScriptFunctor makeClassMember(RET(CLS::*func)(Args...)) {
		auto mf = std::mem_fn(func);
		auto l = [=](CLS* inst, Args... args) {
			return mf(inst, args...);
		};
		return makeClassMember<CLS>(l);
	}

	template<typename CLS, typename RET, typename... Args>
	ScriptFunctor makeClassMember(RET(CLS::*func)(Args...) const) {
		auto mf = std::mem_fn(func);
		auto l = [=](CLS* inst, Args... args) {
			return mf(inst, args...);
		};
		return makeClassMember<CLS>(l);
	}

	// ******************************************************************************************************************************
	// singleton.member
	// ******************************************************************************************************************************

	template<typename CLS, typename RET, typename... Args>
	ScriptFunctor makeSingletonMember(CLS* inst, RET(CLS::*func)(Args...)) {
		auto mf = std::mem_fn(func);
		auto l = [=](Args... args) {
			return mf(inst, args...);
		};
		return makeStatic(l);
	}

	template<typename CLS, typename RET, typename... Args>
	ScriptFunctor makeSingletonMember(CLS* inst, RET(CLS::*func)(Args...) const) {
		auto mf = std::mem_fn(func);
		auto l = [=](Args... args) {
			return mf(inst, args...);
		};
		return makeStatic(l);
	}


	// **************************************************************************************************
	// ScriptFunctorHolder
	// **************************************************************************************************

	class ScriptFunctorHolder {
		public:
			Engine* engine;
			std::string name;
			v8::Persistent<v8::Function> funcPersistent;
			ScriptFunctor functor;

			ScriptFunctorHolder(Engine* engine, const std::string& name, const ScriptFunctor& functor) : engine(engine), name(name), functor(functor) {

			}
			~ScriptFunctorHolder() {
				funcPersistent.Reset();
			}

			void call(const v8::FunctionCallbackInfo<v8::Value>& args) {
				functor(engine, name, args);
			}
	};

	// **************************************************************************************************
	// ScriptFunctionCaller
	// **************************************************************************************************

	class ScriptFunctionCaller;
	typedef stdext::object_ptr<ScriptFunctionCaller> ScriptFunctionCallerPtr;

	class ScriptFunctionCaller: public stdext::object {
		public:
			v8::Persistent<v8::Function> func;
			Engine* engine;

			ScriptFunctionCaller(Engine* engine, v8::Local<v8::Value> f) {
				this->engine = engine;

				if (!f.IsEmpty() && f->IsFunction()) {
					func.Reset(engine->isolate(), v8::Local<v8::Function>::Cast(f));
				}
			}

			virtual ~ScriptFunctionCaller() {
				func.Reset();
			}

			template<typename R, typename... Args>
			R callReturn(Args... args) {
				if (func.IsEmpty()) {
					return R();
				} else {
					ScriptingScope scope(engine);
					v8::Local<v8::Object> self = engine->newAnonymousObject();

					R res = engine->CallScriptFunction<R>(func.Get(engine->isolate()), self, args...);
					scope.checkThrowException();
					return res;
				}
			}

			template<typename... Args>
			void callVoid(Args... args) {
				if (func.IsEmpty()) return;

				ScriptingScope scope(engine);
				v8::Local<v8::Object> self = engine->newAnonymousObject();

				engine->CallScriptFunction<void>(func.Get(engine->isolate()), self, args...);
				scope.checkThrowException();
			}
	};

	template<typename R, typename... Args>
	class ScriptFunctionCallerExecutor {
		public:
			ScriptFunctionCallerPtr caller;

			ScriptFunctionCallerExecutor(ScriptFunctionCallerPtr caller) : caller(caller) { }

			R operator()(Args... args) {
				return caller->callReturn<R, Args...>(args...);
			}
	};

	template<typename... Args>
	class ScriptFunctionCallerExecutor<void,Args...> {
		public:
			ScriptFunctionCallerPtr caller;

			ScriptFunctionCallerExecutor(ScriptFunctionCallerPtr caller) : caller(caller) { }

			void operator()(Args... args) {
				caller->callVoid<Args...>(args...);
			}
	};

	// **************************************************************************************************
	// constructors
	// **************************************************************************************************

	namespace impl {
		template<bool CONSTRUCTIBLE>
		struct ConstructorCallbackInstanceCreator;

		template<>
		struct ConstructorCallbackInstanceCreator<true> {
			template<typename C>
			static bool call(const std::string& prototypeName, Engine* engine, const v8::FunctionCallbackInfo<v8::Value>& args, v8::Local<v8::Value>& ret) {
				C* inst = new C;
				inst->scriptingSetClassNameInternal(prototypeName);
				ret = inst->scriptingGetObject();
				return true;
			}
		};
		template<>
		struct ConstructorCallbackInstanceCreator<false> {
			template<typename C>
			static bool call(const std::string& prototypeName, Engine* engine, const v8::FunctionCallbackInfo<v8::Value>& args, v8::Local<v8::Value>& ret) {
				ret = engine->newUndefined();
				return false;
			}
		};

		template<bool HAS_SCRIPTING_CTOR>
		struct ConstructorCallbackStaticCreator;

		template<>
		struct ConstructorCallbackStaticCreator<true> {
			template<typename C>
			static bool call(const std::string& prototypeName, Engine* engine, const v8::FunctionCallbackInfo<v8::Value>& args, v8::Local<v8::Value>& ret) {
				return callWrapper<C>(prototypeName, engine, args, ret, &C::scriptingCtor);
			}

			template<typename C, typename... Args>
			static bool callWrapper(const std::string& prototypeName, Engine* engine, const v8::FunctionCallbackInfo<v8::Value>& args, v8::Local<v8::Value>& ret, stdext::object_ptr<C> (*ctor)(Args...)) {
				std::tuple<typename stdext::remove_const_ref<Args>::type...> argsTuple = internal::UnmapArgs<typename stdext::remove_const_ref<Args>::type...>(engine, args);
				stdext::object_ptr<C> inst = internal::CallFunctionFromTuple<stdext::object_ptr<C>>(ctor, argsTuple);

				if (inst == nullptr) return false;

				inst->scriptingSetClassNameInternal(prototypeName);
				ret = inst->scriptingGetObject();
				return true;
			}
		};

		template<>
		struct ConstructorCallbackStaticCreator<false> {
			template<typename C>
			static bool call(const std::string& prototypeName, Engine* engine, const v8::FunctionCallbackInfo<v8::Value>& args, v8::Local<v8::Value>& ret) {
				ret = engine->newUndefined();
				return false;
			}
		};

		template<typename C>
		struct has_scriptingCtor {
			template<typename, typename> struct checker { };

			template<typename T>
			static std::true_type test(checker<T, decltype(&T::scriptingCtor)> *);

			template<typename T>
			static std::false_type test(...);

			typedef decltype(test<C>(nullptr)) type;
			static const bool value = std::is_same<std::true_type, decltype(test<C>(nullptr))>::value;
		};

	}

	// ************************************************************************************
	template<typename C>
	void ConstructorCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
		v8::Local<v8::External> data = v8::Local<v8::External>::Cast(args.Data());
		Engine* engine = (Engine*)data->Value();
		std::string prototypeName = converters::ConverterHelper<std::string>::from(engine, args.Callee()->Get(engine->newString("__protoName")));


		if (!args.IsConstructCall()) {
			engine->throwException(stdext::format("Function %s can be called only as constructor", prototypeName));
			return;
		}

		if (engine->m_suppressCtorCallback) {
			args.GetReturnValue().Set(args.This());
		} else {
			v8::Local<v8::Value> returnValue = engine->newUndefined();
			bool res = false;
			res = impl::ConstructorCallbackStaticCreator<impl::has_scriptingCtor<C>::value>::template call<C>(prototypeName, engine, args, returnValue);
			if (!res) {
				res = impl::ConstructorCallbackInstanceCreator<std::is_default_constructible<C>::value>::template call<C>(prototypeName, engine, args, returnValue);
				if (!res) {
					engine->throwException(stdext::format("Cannot create native object of class %s from prototype %s - not default constructible nor scriptingCtor method.", stdext::demangled_name::get<C>().full(), prototypeName));
					return;
				}
			}

			args.GetReturnValue().Set(returnValue);

			if (!returnValue.IsEmpty() && returnValue->IsObject()) {
				auto retObj = v8::Local<v8::Object>::Cast(returnValue);
				internal::callCtorEvent(engine, retObj, args);
			}
		}
	}

} }

#endif /* INCLUDE_SCRIPTING_FUNCTIONWRAPPER_H_ */

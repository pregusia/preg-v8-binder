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

#ifndef INCLUDE_SCRIPTING_INTERNAL_H_
#define INCLUDE_SCRIPTING_INTERNAL_H_

#include <type_traits>

namespace scripting { namespace internal {

	template<typename T>
	struct remove_ref_const {
		typedef typename std::remove_reference<typename std::remove_const<T>::type>::type type;
	};

	template<bool COPY, typename C>
	struct copy_instance { static C* copy(C* org) { return NULL; } };

	template<typename C>
	struct copy_instance<true,C> { static C* copy(C* org) { return new C(*org); } };

	template<typename C>
	struct copy_instance<false,C> { static C* copy(C* org) { return org; } };

	// **************************************************************************************************
	// unmapping args
	// **************************************************************************************************

	namespace impl {
		template<int N, typename... ArgsT>
		struct UnmapArgsImpl {
			static void apply(std::tuple<ArgsT...>& t, Engine* engine, const v8::FunctionCallbackInfo<v8::Value>& args) {
				if (N >= 0 && N < args.Length()) {
					converters::convertFrom(engine, args[N], std::get<N>(t));
				}
				UnmapArgsImpl<N-1,ArgsT...>::apply(t, engine, args);
			}
		};
		template<typename... ArgsT>
		struct UnmapArgsImpl<-1,ArgsT...> {
			static void apply(std::tuple<ArgsT...>& t, Engine* engine, const v8::FunctionCallbackInfo<v8::Value>& args) {

			}
		};
	}

	template<typename... Args>
	std::tuple<Args...> UnmapArgs(Engine* engine, const v8::FunctionCallbackInfo<v8::Value>& args) {
		std::tuple<Args...> t;
		impl::UnmapArgsImpl<(int)(sizeof...(Args))-1,Args...>::apply(t, engine, args);
		return t;
	}

	// **************************************************************************************************
	// mapping args
	// **************************************************************************************************

	namespace impl {
		template<int N>
		struct MapArgsImpl {
			static void apply(Engine* engine, v8::Local<v8::Value>* out) {

			}

			template<typename F, typename... Args>
			static void apply(Engine* engine, v8::Local<v8::Value>* out, F first, Args... args) {
				out[N] = converters::convertTo(engine, first);
				MapArgsImpl<N + 1>::apply(engine, out, args...);
			}
		};
	}

	template<typename... Args>
	void MapArgs(Engine* engine, v8::Local<v8::Value>* out, Args... args) {
		impl::MapArgsImpl<0>::apply(engine, out, args...);
	}

	// **************************************************************************************************
	// calling from tuple
	// **************************************************************************************************

	namespace impl {
		template<int N, typename R>
		struct CallFunctionFromTupleImpl {
			template<typename F, typename... ArgsF, typename... ArgsT>
			static R call(const F& f, const std::tuple<ArgsT...>& t, ArgsF... args) {
				return CallFunctionFromTupleImpl<N-1, R>::call(f, t, std::get<N-1>(t), args...);
			}
		};

		template<typename R>
		struct CallFunctionFromTupleImpl<0,R> {
			template<typename F, typename... ArgsF, typename... ArgsT>
			static R call(const F& f, const std::tuple<ArgsT...>& t, ArgsF... args) {
				return f(args...);
			}
		};

		template<typename R, typename F, typename... Args>
		struct CallFunctionFromTupleMapReturn {
			static v8::Local<v8::Value> call(Engine* engine, const F& f, const std::tuple<Args...>& t, Args... args) {
				R ret = CallFunctionFromTupleImpl<sizeof...(Args), R>::call(f, t);
				return converters::convertTo(engine, ret);
			}
		};

		template<typename F, typename... Args>
		struct CallFunctionFromTupleMapReturn<void, F, Args...> {
			static v8::Local<v8::Value> call(Engine* engine, const F& f, const std::tuple<Args...>& t, Args... args) {
				CallFunctionFromTupleImpl<sizeof...(Args), void>::call(f, t);
				return engine->newUndefined();
			}
		};
	}

	template<typename R, typename F, typename... Args>
	R CallFunctionFromTuple(const F& f, const std::tuple<Args...>& t) {
		return impl::CallFunctionFromTupleImpl<sizeof...(Args), R>::call(f, t);
	}

	template<typename R, typename F, typename... Args>
	v8::Local<v8::Value> CallFunctionFromTupleMapReturn(Engine* engine, const F& f, const std::tuple<Args...>& t) {
		return impl::CallFunctionFromTupleMapReturn<R, F, Args...>::call(engine, f, t);
	}

	// **************************************************************************************************
	// calling class from tuple
	// **************************************************************************************************

	namespace impl {
		template<int N, typename R>
		struct CallClassFunctionFromTupleImpl {
			template<typename C, typename F, typename... ArgsF, typename... ArgsT>
			static R call(C* inst, const F& f, const std::tuple<ArgsT...>& t, ArgsF... args) {
				return CallClassFunctionFromTupleImpl<N-1, R>::call(inst, f, t, std::get<N-1>(t), args...);
			}
		};

		template<typename R>
		struct CallClassFunctionFromTupleImpl<0,R> {
			template<typename C, typename F, typename... ArgsF, typename... ArgsT>
			static R call(C* inst, const F& f, const std::tuple<ArgsT...>& t, ArgsF... args) {
				return (inst->*f)(args...);
			}
		};

		template<typename R>
		struct CallClassFunctionFromTupleMapReturn {
			template<typename C, typename F, typename... Args>
			static v8::Local<v8::Value> call(Engine* engine, C* inst, const F& f, const std::tuple<Args...>& t) {
				R ret = CallClassFunctionFromTupleImpl<sizeof...(Args),R>::call(inst, f, t);
				return converters::convertTo(engine, ret);
			}
		};

		template<>
		struct CallClassFunctionFromTupleMapReturn<void> {
			template<typename C, typename F, typename... Args>
			static v8::Local<v8::Value> call(Engine* engine, C* inst, const F& f, const std::tuple<Args...>& t) {
				CallClassFunctionFromTupleImpl<sizeof...(Args),void>::call(inst, f, t);
				return engine->newUndefined();
			}
		};
	}

	template<typename R, typename C, typename F, typename... Args>
	R CallClassFunctionFromTuple(C* inst, const F& f, const std::tuple<Args...>& t) {
		return impl::CallClassFunctionFromTupleImpl<sizeof...(Args),R>::call(inst, f, t);
	}

	template<typename R, typename C, typename F, typename... Args>
	v8::Local<v8::Value> CallClassFunctionFromTupleMapReturn(Engine* engine, C* inst, const F& f, const std::tuple<Args...>& t) {
		return impl::CallClassFunctionFromTupleMapReturn<R>::template call<C,F,Args...>(engine, inst, f, t);
	}

	// **************************************************************************************************
	// set array values
	// **************************************************************************************************

	namespace impl {
		struct SetArrayValuesImpl {
			static void apply(Engine* engine, v8::Local<v8::Array>& arr, int n) { }

			template<typename F, typename... Args>
			static void apply(Engine* engine, v8::Local<v8::Array>& arr, int n, const F& f, Args... args) {
				arr->Set(n, converters::convertTo(engine, f));
				SetArrayValuesImpl::apply(arr, n + 1, args...);
			}
		};
	}

	template<typename... Args>
	void SetArrayValues(Engine* engine, v8::Local<v8::Array>& arr, Args... args) {
		impl::SetArrayValuesImpl::apply(engine, arr, 0, args...);
	}

	// **************************************************************************************************
	// set object props
	// **************************************************************************************************

	namespace impl {
		struct SetObjectPropsImpl {
			static void apply(Engine* engine, v8::Local<v8::Object>& obj) {

			}
			template<typename K, typename V, typename... Args>
			static void apply(Engine* engine, v8::Local<v8::Object>& obj, const K& k, const V& v, Args... args) {
				obj->DefineOwnProperty(
					engine->context(),
					engine->newString(k),
					converters::convertTo(engine, v)
				);
				SetObjectPropsImpl::apply(engine, obj, args...);
			}
		};
	}

	template<typename... Args>
	void SetObjectProps(Engine* engine, v8::Local<v8::Object>& obj, Args... args) {
		impl::SetObjectPropsImpl::apply(engine, obj, args...);
	}

	bool SetObjectPropChain(Engine* engine, v8::Local<v8::Object> obj, const std::string& name, const v8::Local<v8::Value>& val);
	std::string normalizePrototypeName(const std::string& name, const std::string& ns);
	void callCtorEvent(Engine* engine, v8::Local<v8::Object>& obj, const v8::FunctionCallbackInfo<v8::Value>& args);

} }

#endif /* INCLUDE_SCRIPTING_INTERNAL_H_ */

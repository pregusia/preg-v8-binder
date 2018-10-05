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


#ifndef INCLUDE_SCRIPTING_OBJECT_H_
#define INCLUDE_SCRIPTING_OBJECT_H_

#include "base.h"
#include <v8.h>
#include "utils.h"

namespace scripting {

	class Engine;

	class ScriptableObject : public virtual stdext::object {
		public:
			ScriptableObject();
			virtual ~ScriptableObject();
			bool scriptingEnabled() const { return true; }

			template<typename R, typename... Args>
			R scriptingCallMethod(const std::string& name, Args... args);

			template<typename... Args>
			bool scriptingCallEvent(const std::string& name, Args... args);

			template<typename RET, typename... Args>
			std::vector<RET> scriptingCallEventReturn(const std::string& name, Args... args);

			template<typename R>
			R scriptingGetField(const std::string& name1, const std::string& name2 = "", const std::string& name3 = "");

			v8::Local<v8::Object> scriptingGetObject();
			const std::string& scriptingClassName() const { return m_scriptingClassName; }
			void scriptingSetClassNameInternal(const std::string& s, int32_t usedMemory = 1024);

			virtual void eventRegister(const std::string& name) { }
			virtual void eventUnregister(const std::string& name) { }

			template<typename C>
			static C* unwrap(v8::Local<v8::Value> val) {
				if (val.IsEmpty()) {
					utils::logWarning(stdext::format("Unwrapping object of class %s failed - val.empty", stdext::demangled_name::get<C>().full()));
					return nullptr;
				}
				if (!val->IsObject()) {
					utils::logWarning(stdext::format("Unwrapping object of class %s failed - not object", stdext::demangled_name::get<C>().full()));
					return nullptr;
				}
				v8::Local<v8::Object> obj = v8::Local<v8::Object>::Cast(val);
				if (obj->InternalFieldCount() == 0) {
					utils::logWarning(stdext::format("Unwrapping object of class %s failed - ifc == 0", stdext::demangled_name::get<C>().full()));
					return nullptr;
				}

				stdext::object_ptr<ScriptableObject>* ptr = (stdext::object_ptr<ScriptableObject>*)v8::Local<v8::External>::Cast(obj->GetInternalField(0))->Value();
				if (ptr == nullptr) {
					utils::logWarning(stdext::format("Unwrapping object of class %s failed - ptr NULL", stdext::demangled_name::get<C>().full()));
					return nullptr;
				}

				return dynamic_cast<C*>(ptr->get());
			}


		protected:
			v8::Persistent<v8::Object> m_scriptingObject;
			int32_t m_scriptingMemory;
			std::string m_scriptingClassName;

			static void freeCallback(const v8::WeakCallbackInfo<void>& info);
	};

} /* namespace scripting */

#include "engine.h"

namespace scripting {

	// ************************************************************************************
	template<typename R, typename... Args>
	R ScriptableObject::scriptingCallMethod(const std::string& name, Args... args) {
		ScriptingScope scope(g_engineScripting);

		v8::Local<v8::Object> obj = scriptingGetObject();
		assert(!obj.IsEmpty());
		return g_engineScripting->CallObjectProperty<R>(obj, name, args...);
		// TODO: check exception somehow
	}

	// ************************************************************************************
	template<typename R>
	R ScriptableObject::scriptingGetField(const std::string& name1, const std::string& name2, const std::string& name3) {
		ScriptingScope scope(g_engineScripting);

		v8::Local<v8::Value> val;
		v8::Local<v8::Object> obj = scriptingGetObject();
		if (obj.IsEmpty()) return R();

		if (!name1.empty() && !obj.IsEmpty()) {
			val = obj->Get(g_engineScripting->newString(name1));
			if (!val.IsEmpty() && val->IsObject()) obj = v8::Local<v8::Object>::Cast(val);
		}

		if (!name2.empty() && !obj.IsEmpty()) {
			val = obj->Get(g_engineScripting->newString(name2));
			if (!val.IsEmpty() && val->IsObject()) obj = v8::Local<v8::Object>::Cast(val);
		}

		if (!name3.empty() && !obj.IsEmpty()) {
			val = obj->Get(g_engineScripting->newString(name3));
			if (!val.IsEmpty() && val->IsObject()) obj = v8::Local<v8::Object>::Cast(val);
		}

		R ret = R();
		converters::convertFrom(g_engineScripting, val, ret);
		return ret;
	}

	// ************************************************************************************
	template<typename... Args>
	bool ScriptableObject::scriptingCallEvent(const std::string& name, Args... args) {
		//utils::logDebug(stdext::format("Calling event %s on [%s %s]", name, m_scriptingClassName, stdext::demangle_name(typeid(*this).name())));
		ScriptingScope scope(g_engineScripting);

		v8::Local<v8::Function> dollarFunc = g_engineScripting->getGlobalFunction("$");
		if (!dollarFunc.IsEmpty()) {
			bool res = g_engineScripting->CallObjectProperty<bool>(dollarFunc, "eventCallStatic", scriptingGetObject(), name, args...);
			scope.checkThrowException();
			return res;
		}

		return false;
	}

	// ************************************************************************************
	template<typename RET, typename... Args>
	std::vector<RET> ScriptableObject::scriptingCallEventReturn(const std::string& name, Args... args) {
		//utils::logDebug(stdext::format("Calling event %s on [%s %s]", name, m_scriptingClassName, stdext::demangle_name(typeid(*this).name())));
		ScriptingScope scope(g_engineScripting);

		v8::Local<v8::Function> dollarFunc = g_engineScripting->getGlobalFunction("$");
		if (!dollarFunc.IsEmpty()) {
			std::vector<RET> res = g_engineScripting->CallObjectProperty<std::vector<RET>>(dollarFunc, "eventCallStaticReturn", scriptingGetObject(), name, args...);
			scope.checkThrowException();
			return res;
		}

		return std::vector<RET>();
	}

}

#endif /* INCLUDE_SCRIPTING_OBJECT_H_ */


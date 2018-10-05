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

#include "object.h"
#include "engine.h"

namespace scripting {

	// ************************************************************************************
	ScriptableObject::ScriptableObject() {
		m_scriptingMemory = 1024;
	}

	// ************************************************************************************
	ScriptableObject::~ScriptableObject() {
		assert(m_scriptingObject.IsEmpty());
	}

	// ************************************************************************************
	void ScriptableObject::scriptingSetClassNameInternal(const std::string& s, int32_t usedMemory) {
		m_scriptingMemory = usedMemory;
		if (s != m_scriptingClassName && m_scriptingObject.IsEmpty()) {
			m_scriptingClassName = s;
		}
	}

	// ************************************************************************************
	v8::Local<v8::Object> ScriptableObject::scriptingGetObject() {
		if (m_scriptingClassName.empty()) {
			utils::logWarning("Tried to get scriptingObject from not initialized instance.");
			return v8::Local<v8::Object>();
		}

		if (m_scriptingObject.IsEmpty()) {
			v8::Local<v8::Object> obj = g_engineScripting->newObjectFromPrototype(m_scriptingClassName,
				"__scriptingClassName", m_scriptingClassName,
				"__nativeClassName", stdext::demangled_name::createFromString(typeid(*this).name()).full()
			);

			assert(obj->InternalFieldCount() > 0);
			stdext::object_ptr<ScriptableObject>* ptr = new stdext::object_ptr<ScriptableObject>(dynamic_self_cast<ScriptableObject>());
			obj->SetInternalField(0, g_engineScripting->newExternal(ptr));

			m_scriptingObject.Reset(g_engineScripting->isolate(), obj);
			g_engineScripting->isolate()->AdjustAmountOfExternalAllocatedMemory(m_scriptingMemory);

			__refsInc();

			/*
			utils::logDebug(stdext::format("Mapping object #%d of class %s [Native %s] to scripting. refs=%d",
				__objectId(),
				m_scriptingClassName,
				stdext::demangle_name(typeid(*this).name()),
				__refsCount()
			));
			*/

			m_scriptingObject.SetWeak((void*)ptr,&freeCallback, v8::WeakCallbackType::kParameter);
		}

		return m_scriptingObject.Get(g_engineScripting->isolate());
	}

	// ************************************************************************************
	void ScriptableObject::freeCallback(const v8::WeakCallbackInfo<void>& info) {
		stdext::object_ptr<ScriptableObject>* ptr = static_cast<stdext::object_ptr<ScriptableObject>*>(info.GetParameter());
		(*ptr)->m_scriptingObject.Reset();
		g_engineScripting->isolate()->AdjustAmountOfExternalAllocatedMemory((*ptr)->m_scriptingMemory);
		delete ptr;
	}

} /* namespace scripting */


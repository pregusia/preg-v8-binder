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



#include <scripting/base.h>
#include <scripting/object.h>
#include <cstdio>

// class pre-definition
class TestClass;
typedef stdext::object_ptr<TestClass> TestClassPtr;


// test class definition

class TestClass: public scripting::ScriptableObject {
	public:

		TestClass(int32_t val) : m_value(val) {
			// have to set className before using in scripting engine
			scriptingSetClassNameInternal("testing.TestClass", 16);
		}

		// constructor called from JS
		static TestClassPtr scriptingCtor(int32_t val) { return new TestClass(val); }

		void add(int32_t val) { m_value += val; }
		void sub(int32_t val) { m_value -= val; }

		std::string asString() const { return stdext::format("%d", m_value); }
		int32_t asInt() const { return m_value; }

		void set(int32_t val) { m_value = val; }

		void callTestEvent() {
			scriptingCallEvent("onTestEvent", m_value);
		}

	private:
		int32_t m_value;

};


int main() {

	g_engineScripting = new scripting::Engine;

	// registering native class to V8
    g_engineScripting->registerNativeClass<TestClass>("testing");

    // registering member functions
    g_engineScripting->registerNativeClassMemberFunction<TestClass>("add", &TestClass::add);
    g_engineScripting->registerNativeClassMemberFunction<TestClass>("sub", &TestClass::sub);
    g_engineScripting->registerNativeClassMemberFunction<TestClass>("asString", &TestClass::asString);
    g_engineScripting->registerNativeClassMemberFunction<TestClass>("asInt", &TestClass::asInt);

    // registering property accesor
    g_engineScripting->registerNativeClassPropertyAccessor<TestClass>("value",
    	[](TestClass* obj) -> int32_t { return obj->asInt(); },
		[](TestClass* obj, int32_t val) -> void { obj->set(val); }
    );

    // registering some native static function
    g_engineScripting->registerGlobalStaticFunction("testGlobalFunc", [](int32_t a, int32_t b) -> int32_t { return a + b; });


    // allocating new TestClass object, and global function to return native-instance to V8
    TestClassPtr someObject(new TestClass(0));
    g_engineScripting->registerGlobalStaticFunction("getSomeObjectInstance", [&]() -> TestClassPtr { return someObject; });

    // executing file test.js
    g_engineScripting->runFile("./test.js");


    // > event testing
    // event was registered in test.js
    // now we can call is from C++
    someObject->callTestEvent();





	return 0;
}



preg-v8-binder
======

Library for binding c++ classes/methods to Google V8 engine in OOP way.
Inspired by OTClient lua engine (by edubart, https://github.com/edubart/otclient).

Requeriments
------------------

- C++11 support
- C++ ABI for demangling class names
- RTTI enabled
- google V8 Engine, at least 5.8 version (tested on 5.8 version)


Limitations
-----------

- Multiple inheritance is not supported
- Objects must be derived from stdext::object


Features
-----------

- Bind C++ classes to v8
- Support for std::function and lambdas
- Events system
- JS 'namespaces' support


Examples
-----------

available in examples directory.


Usage
-----------

Copy-paste into your destination project, link agains google V8 and use.



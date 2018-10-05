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


#ifndef INCLUDE_SCRIPTING_BASE_H_
#define INCLUDE_SCRIPTING_BASE_H_

// common C headers
#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <cmath>

// common STL headers
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <deque>
#include <map>
#include <set>
#include <algorithm>
#include <functional>
#include <array>
#include <unordered_map>
#include <tuple>
#include <iomanip>
#include <typeinfo>

#include <stdext/stdext.h>

namespace scripting { class Engine; }
extern scripting::Engine* g_engineScripting;

typedef std::vector<std::string> StringVector;
typedef std::list<std::string> StringList;

typedef std::vector<int32_t> Int32Vector;
typedef std::list<int32_t> Int32List;
typedef std::set<int32_t> Int32Set;

typedef std::vector<uint32_t> UInt32Vector;
typedef std::list<uint32_t> UInt32List;
typedef std::set<uint32_t> UInt32Set;


#endif /* INCLUDE_SCRIPTING_BASE_H_ */

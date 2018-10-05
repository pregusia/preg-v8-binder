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


#include "utils.h"

#include <cstdio>

namespace scripting { namespace utils {

	// XXX: sample utils file,
	// XXX: should be customized to match destination project logging system etc

	// ************************************************************************************
	void logDebug(const std::string& msg) {
		printf("[logDebug] %s\n", msg.c_str());
	}

	// ************************************************************************************
	void logInfo(const std::string& msg) {
		printf("[logInfo] %s\n", msg.c_str());
	}

	// ************************************************************************************
	void logError(const std::string& msg) {
		printf("[logError] %s\n", msg.c_str());
	}

	// ************************************************************************************
	void logWarning(const std::string& msg) {
		printf("[logWarning] %s\n", msg.c_str());
	}

	// ************************************************************************************
	void logScript(const std::string& msg) {
		printf("[logScript] %s\n", msg.c_str());
	}

	// ************************************************************************************
	std::string readFileContents(const std::string& path) {
		FILE* fp = fopen(path.c_str(), "r");
		std::string content;

		if (fp != nullptr) {
			size_t size = 0;
			fseek(fp, 0, SEEK_END);
			size = ftell(fp);
			fseek(fp, 0, SEEK_SET);

			if (size > 0) {
				char* buf = new char[size];
				fread(buf, 1, size, fp);
				content = std::string(buf, size);
				delete[] buf;
			}

			fclose(fp);
		}

		return content;
	}

	// ************************************************************************************
	StringVector split(const std::string& str, char separator) {
		if (str.empty()) return StringVector();

		StringVector res;
		std::string curr;
		for(size_t i=0;i<str.length();++i) {
			if (str[i] == separator) {
				res.push_back(curr);
				curr.clear();
			} else {
				curr.push_back(str[i]);
			}
		}
		if (!curr.empty()) res.push_back(curr);

		return res;
	}


} }


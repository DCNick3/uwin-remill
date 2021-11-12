// Copyright (C) 2020 Trail of Bits
// Based on: https://github.com/andrew-hardin/cmake-git-version-tracking/blob/master/better-example/git.cc.in
// Which is (C) 2020 Andrew Hardin
// 
// MIT License
//  Copyright (c) 2020 Andrew Hardin
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#include "remill/Version/Version.h"

namespace remill {
namespace version {

bool HasVersionData(void) {
  return false;
}

bool HasUncommittedChanges(void) {
  return false;
}

std::string_view GetAuthorName(void) {
  return "";
}

std::string_view GetAuthorEmail(void) {
  return "";
}

std::string_view GetCommitHash(void) {
  return "";
}

std::string_view GetCommitDate(void) {
  return "";
}

std::string_view GetCommitSubject(void) {
  return "";
}

std::string_view GetCommitBody(void) {
  return "";
}

std::string_view GetVersionString(void) {
  return "";
}

}  // namespace version
}  // namespace remill

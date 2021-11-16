// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <cassert>

#include <gflags/gflags.h>

DEFINE_string(identifier, "resources",
          "name of the resources function");
DEFINE_string(output_header, "", "output header file");
DEFINE_string(output_impl, "", "output cc impl file");
DEFINE_string(cpp_namespace, "", "generate in a c++ namespace");
DEFINE_string(strip_prefix, "", "strip prefix from filenames");
DEFINE_bool(flatten, false,
          "whether to flatten the directory structure (only include basename)");

std::vector<std::string> StrSplit(std::string s, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find (delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}

void GenerateNamespaceOpen(std::ofstream& f) {
  const auto& ns = FLAGS_cpp_namespace;
  if (ns.empty()) return;

  std::vector<std::string> ns_comps = StrSplit(FLAGS_cpp_namespace, "::");
  for (const auto& ns_comp : ns_comps) {
    f << "namespace " << ns_comp << " {\n";
  }
}

void GenerateNamespaceClose(std::ofstream& f) {
  const auto& ns = FLAGS_cpp_namespace;
  if (ns.empty()) return;

  std::vector<std::string> ns_comps = StrSplit(FLAGS_cpp_namespace, "::");
  for (size_t i = 0, e = ns_comps.size(); i < e; ++i) {
    f << "}\n";
  }
}

void GenerateTocStruct(std::ofstream& f) {
  f << "struct FileToc {\n";
  f << "  const char* name;             // the file's original name\n";
  f << "  const char* data;             // beginning of the file\n";
  f << "  std::size_t size;             // length of the file\n";
  f << "};\n";
}

bool GenerateHeader(const std::string& header_file,
                    const std::vector<std::string>& toc_files) {
  std::ofstream f(header_file, std::ios::out | std::ios::trunc);
  f << "#pragma once\n";  // Pragma once isn't great but is the best we can do.
  f << "#include <cstddef>\n";
  GenerateNamespaceOpen(f);
  GenerateTocStruct(f);
  f << "extern const struct FileToc* " << FLAGS_identifier
    << "_create();\n";
  f << "static std::size_t " << FLAGS_identifier
    << "_size() { \n";
  f << "  return " << toc_files.size() << ";\n";
  f << "}\n";
  GenerateNamespaceClose(f);
  f.close();
  return f.good();
}

bool SlurpFile(const std::string& file_name, std::string* contents) {
  constexpr std::streamoff kMaxSize = 100000000;
  std::ifstream f(file_name, std::ios::in | std::ios::binary);
  // get length of file:
  f.seekg(0, f.end);
  std::streamoff length = f.tellg();
  f.seekg(0, f.beg);
  if (!f.good()) return false;

  if (length > kMaxSize) {
    std::cerr << "File " << file_name << " is too large\n";
    return false;
  }

  size_t mem_length = static_cast<size_t>(length);
  contents->resize(mem_length);
  f.read(&(*contents)[0], mem_length);
  f.close();
  return f.good();
}

std::string CEscape(std::string const& s) {
  std::string res;
  for (unsigned char c : s) {
    switch (c) {
      case '\\':
      case '"':
        res += '\\';
        res += c;
        break;
      case '\n':
        res += "\n"; break;
      case '\r':
        res += "\r"; break;
      case '\t':
        res += "\t"; break;
      case '\'':
        res += "\'"; break;
      default:
        if (c >= 0x20 && c < 0x7f) {
          res += c;
        } else {
          res += '\\';
          res += '0' + c / 64;
          res += '0' + (c % 64) / 8;
          res += '0' + c % 8;
        }
    }
  }
  return res;
}

std::string CHexEscape(std::string const& s) {
  static char hexChars[17] = "0123456789abcdef";

  std::string res;
  bool last_hex_escape = false;  // true if last output char was \xNN.
  for (unsigned char c : s) {
    bool is_hex_escape = false;
    switch (c) {
      case '\\':
      case '"':
        res += '\\';
        res += c;
        break;
      case '\n':
        res += "\\n"; break;
      case '\r':
        res += "\\r"; break;
      case '\t':
        res += "\\t"; break;
      case '\'':
        res += "\\'"; break;
      default:
        // Note that if we emit \xNN and the src character after that is a hex
        // digit then that digit must be escaped too to prevent it being
        // interpreted as part of the character code by C.
        if (c >= 0x20 && c < 0x7f && !(last_hex_escape && c >= '0' && c <= '9')) {
          res += c;
        } else {
          res += '\\';
          res += 'x';
          res += hexChars[c / 16];
          res += hexChars[c % 16];
          is_hex_escape = true;
        }
    }
    last_hex_escape = is_hex_escape;
  }
  return res;
}

std::string StripPrefix(std::string const& s, std::string const& prefix) {
  std::terminate();
}

bool GenerateImpl(const std::string& impl_file,
                  const std::vector<std::string>& input_files,
                  const std::vector<std::string>& toc_files) {
  std::ofstream f(impl_file, std::ios::out | std::ios::trunc);
  f << "#include <cstddef>\n";
  GenerateNamespaceOpen(f);
  GenerateTocStruct(f);
  f << "static const struct FileToc toc[] = {\n";
  assert(input_files.size() == toc_files.size());
  for (size_t i = 0, e = input_files.size(); i < e; ++i) {
    f << "  {";
    f << "\"" << CEscape(toc_files[i]) << "\", ";
    std::string contents;
    if (!SlurpFile(input_files[i], &contents)) {
      std::cerr << "Error reading file " << input_files[i] << "\n";
      return false;
    }
    f << "\"" << CHexEscape(contents) << "\\0\", ";
    f << contents.size() << "},\n";
  }
  f << "  {nullptr, nullptr, 0},\n";
  f << "};\n";
  f << "const struct FileToc* " << FLAGS_identifier
    << "_create() {\n";
  f << "  return &toc[0];\n";
  f << "}\n";

  GenerateNamespaceClose(f);
  f.close();
  return f.good();
}

int main(int argc, char** argv) {
  // Parse flags.
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  std::vector<char*> raw_positional_args(argv, argv + argc);
  std::vector<std::string> input_files;
  input_files.reserve(raw_positional_args.size() - 1);
  // Skip program name.
  for (size_t i = 1, e = raw_positional_args.size(); i < e; ++i) {
    input_files.push_back(std::string(raw_positional_args[i]));
  }

  // Generate TOC files by optionally removing a prefix.
  std::vector<std::string> toc_files;
  toc_files.reserve(input_files.size());
  const std::string& strip_prefix = FLAGS_strip_prefix;
  for (const auto& input_file : input_files) {
    std::string toc_file = input_file;
    if (!strip_prefix.empty()) {
      toc_file = std::string(StripPrefix(toc_file, strip_prefix));
    }
    if (FLAGS_flatten) {
      std::vector<std::string> comps = StrSplit(toc_file, "/");
      toc_file = comps.back();
    }
    toc_files.push_back(toc_file);
  }

  if (!FLAGS_output_header.empty()) {
    if (!GenerateHeader(FLAGS_output_header, toc_files)) {
      std::cerr << "Error generating headers.\n";
      return 1;
    }
  }

  if (!FLAGS_output_impl.empty()) {
    if (!GenerateImpl(FLAGS_output_impl, input_files,
                      toc_files)) {
      std::cerr << "Error generating impl.\n";
      return 2;
    }
  }

  return 0;
}
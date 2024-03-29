# Copyright 2019 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Generates source files with embedded file contents.

load(":build_defs.bzl", "cc_embed_data")

package(
    default_visibility = ["//visibility:public"],
    licenses = ["notice"],  # Apache 2.0
)

py_binary(
  name = "generate_embed",
  srcs = ["embed.py"],
  main = "embed.py",
)

cc_embed_data(
    name = "testembed1",
    srcs = [
        "file1.txt",
        "data/file2.txt",
    ],
    cpp_namespace = "foobar",
    h_file_output = "testembed1.h",
)

cc_test(
    name = "testembed1_test",
    srcs = ["testembed1_test.cc"],
    deps = [
        ":testembed1",
        "@com_google_googletest//:gtest_main",
    ],
)
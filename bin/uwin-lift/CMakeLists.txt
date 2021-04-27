# Copyright (c) 2017 Trail of Bits, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

project(uwin-lift)
cmake_minimum_required(VERSION 3.2)

#
# target settings
#

set(UWIN_LIFT uwin-lift-${REMILL_LLVM_VERSION})

set(INTRINSICS_SRC "${CMAKE_CURRENT_LIST_DIR}/intrinsics.cpp")
set(INTRINSICS_BC "${CMAKE_CURRENT_BINARY_DIR}/intrinsics.bc")

add_custom_command(OUTPUT "${INTRINSICS_BC}"
        COMMAND "${CMAKE_BC_COMPILER}" "-I${CMAKE_SOURCE_DIR}/include" -emit-llvm -funwind-tables -c "${INTRINSICS_SRC}" -o "${INTRINSICS_BC}"
        MAIN_DEPENDENCY "${INTRINSICS_SRC}"
        COMMENT "Building BC object ${INTRINSICS_BC}"
        )

add_executable(${UWIN_LIFT}
        Lift.cpp
        "${INTRINSICS_BC}"
        )

target_compile_definitions(${UWIN_LIFT} PRIVATE
        "-DINTRINSICS_BC=\"${INTRINSICS_BC}\"")

#
# target settings
#

target_link_libraries(${UWIN_LIFT} PRIVATE remill)
target_include_directories(${UWIN_LIFT} SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

if(DEFINED WIN32)
  set(install_folder "${CMAKE_INSTALL_PREFIX}/remill")
else()
  set(install_folder "${CMAKE_INSTALL_PREFIX}")
endif()

install(
  TARGETS ${UWIN_LIFT}
  RUNTIME DESTINATION "${install_folder}/bin"
  LIBRARY DESTINATION "${install_folder}/lib"
)

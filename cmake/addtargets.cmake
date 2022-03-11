# MIT License
#
# Copyright © 2021 Rick de Bondt
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# addtargets.cmake - Adds targets in targets list to projects
# Usage: make a target list, then run include(addtargets).

foreach (target IN ITEMS ${targets})
  add_executable(${target} ${${target}_SOURCES})
  target_include_directories(${target} PRIVATE ${${target}_INCLUDE_FOLDERS})
  target_link_libraries(${target} PRIVATE ${${target}_LIBRARIES})
  
  if (${target}_COMPILE_DEFINITIONS)
    target_compile_definitions(${target} PRIVATE ${${target}_COMPILE_DEFINITIONS})
  endif ()
  
  if (${target}_DEPENDENCIES)
    add_dependencies(${target} ${${target}_DEPENDENCIES})
  endif ()

  # Fix formatting
  include(runclangformat)
  target_clangformat_setup(${target})

  # Testing stuff
  if (${target}_TEST_SOURCES AND ENABLE_TESTS)
    # Add all sources to the unittests

    list(APPEND TMPSOURCES ${${target}_SOURCES})
    list(FILTER TMPSOURCES EXCLUDE REGEX ".*${target}.cpp*")
    list(FILTER TMPSOURCES EXCLUDE REGEX "main.cpp")

    add_executable(${target}_tests ${${target}_TEST_SOURCES} ${TMPSOURCES})
    target_include_directories(${target}_tests PRIVATE ${${target}_INCLUDE_FOLDERS} ${${target}_TEST_INCLUDE_FOLDERS})
    target_link_libraries(${target}_tests gtest gmock ${${target}_LIBRARIES})

    if (${target}_COMPILE_DEFINITIONS)
      target_compile_definitions(${target}_tests ${${target}_COMPILE_DEFINITIONS})
    endif ()

    if (${target}_DEPENDENCIES)
      add_dependencies(${target}_tests ${${target}_DEPENDENCIES})
    endif ()

    target_clangformat_setup(${target}_tests)
    gtest_discover_tests(${target}_tests)

    if (AUTO_FORMAT)
      add_dependencies(${target}_tests ${target}_clangformat)
    endif ()
  endif ()

  if (AUTO_FORMAT)
    add_dependencies(${target} ${target}_clangformat)
  endif ()
endforeach ()

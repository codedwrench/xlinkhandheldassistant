# MIT License
#
# Copyright (c) 2020 Aiden Woodruff
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

# Modifications:
# MIT License
#
# Copyright © 2022 Rick de Bondt
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


set(CURSES_INCLUDE_DIRS ${PDCURSES_ROOT_DIR})

# Within XLHA we use wincon
set(CURSES_LIBRARIES ${PDCURSES_ROOT_DIR}/wincon)

find_path(CURSES_INCLUDE_DIRS curses.h)

if (NOT BUILD_STATIC)
    find_library(CURSES_LIBRARIES NAMES pdcurses)
else()
  if(MSVC)
    # Visual Studio does not use .a files
    find_library(CURSES_LIBRARIES NAMES pdcurses.lib)
  else()
    find_library(CURSES_LIBRARIES NAMES pdcurses.a)
  endif()
endif()

file(STRINGS "${CURSES_INCLUDE_DIRS}/curses.h" _CURSES_VERSION_STR REGEX "^#define PDC_VERDOT[ \t]+\\\".*\\\"$")

string(REGEX REPLACE "^#define PDC_VERDOT[ \t]+\\\"\\(.*\\)\\\"$" "\\1" CURSES_VERSION "${_CURSES_VERSION_STR}")

set(PDCURSES_LIBRARIES ${CURSES_LIBRARIES})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
PDCurses
REQUIRED_VARS
  CURSES_LIBRARIES
  CURSES_INCLUDE_DIRS
VERSION_VAR CURSES_VERSION)

set(HAVE_CURSES_H TRUE)

mark_as_advanced(CURSES_LIBRARIES PDCURSES_LIBRARIES CURSES_INCLUDE_DIRS CURSES_VERSION)

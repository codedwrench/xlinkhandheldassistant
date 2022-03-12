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

# addsources.cmake - Adds sources from subdir to project.
# Usage: set TARGET_NAME in parent module, then run include(addsources).

# Adds cpp and h files to TARGET_NAME
file(GLOB TMPSOURCES CONFIGURE_DEPENDS "*.h" "*.cpp" "*.mm")

# Remove unrelated source files
if (TMPSOURCES)
    if(UNIX AND NOT APPLE)
        list(FILTER TMPSOURCES EXCLUDE REGEX ".*Windows.*")
        list(FILTER TMPSOURCES EXCLUDE REGEX ".*Apple.*")
    elseif(UNIX AND APPLE)
        list(FILTER TMPSOURCES EXCLUDE REGEX ".*Windows.*")
        list(FILTER TMPSOURCES EXCLUDE REGEX ".*Linux.*")
    elseif(WIN32)
        list(FILTER TMPSOURCES EXCLUDE REGEX ".*Linux.*")
        list(FILTER TMPSOURCES EXCLUDE REGEX ".*Apple.*")
    endif()
endif ()

list(APPEND ${TARGET_NAME}_SOURCES ${TMPSOURCES})
set(${TARGET_NAME}_SOURCES ${${TARGET_NAME}_SOURCES} PARENT_SCOPE)

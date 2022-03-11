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

# addincludefolder.cmake - Adds include foldder to TARGET_NAME_INCLUDE_FOLDERS
# Usage: set TARGET_NAME in parent module, then run include(addincludefolder).

# Get relative path to cmakelists file, then strip filename, that is our target dir
file(RELATIVE_PATH TARGET_DIR ${PROJECT_SOURCE_DIR} ${CMAKE_PARENT_LIST_FILE})
get_filename_component(TARGET_DIR ${TARGET_DIR} DIRECTORY)

# Add folder to include folders
list(APPEND TMP_INCLUDE_FOLDERS ${${TARGET_NAME}_INCLUDE_FOLDERS})
list(APPEND TMP_INCLUDE_FOLDERS ${TARGET_DIR})
SET(${TARGET_NAME}_INCLUDE_FOLDERS ${TMP_INCLUDE_FOLDERS} PARENT_SCOPE)

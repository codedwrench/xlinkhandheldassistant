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

# obtaingitversion.cmake - Gets git version and puts it in GIT_VERSION variable.

set(GIT_VERSION unknown)
execute_process(COMMAND git describe --abbrev=7 --always --tags OUTPUT_VARIABLE GIT_VERSION)

# Strip trailing stuff
string(REGEX REPLACE "[,\n]" "" GIT_VERSION "${GIT_VERSION}")

message(STATUS "Git version: ${GIT_VERSION}")

# Add as a compiler definition
add_definitions(-DGIT_VERSION="${GIT_VERSION}")

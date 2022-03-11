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

# wrapfindboost.cmake - Finds boost under our terms.
# Usage: Set Boost_ARCHITECTURE if the default arch is not OK, set BOOST_ROOT as well.

set(Boost_NO_SYSTEM_PATHS ON)
set(Boost_USE_STATIC_LIBS ON)

# Needed because otherwise the libraries cannot be found.
if (BUILD_X32)
	set(Boost_ARCHITECTURE "-x32")
else ()
	set(Boost_ARCHITECTURE "-x64")
endif ()

# Concepts seems to break on a bunch of compilers, see: https://github.com/chriskohlhoff/asio/issues/738
add_definitions(-DBOOST_ASIO_DISABLE_CONCEPTS)

find_package(Boost 1.76 REQUIRED COMPONENTS program_options)



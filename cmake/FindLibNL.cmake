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

find_path(LibNL_INCLUDE_DIR netlink/netlink.h HINTS
	${LibNL_ROOT_DIR}/include
	/usr/include
	/usr/include/libnl3
	/usr/local/include
	/usr/local/include/libnl3
)

if (BUILD_STATIC)
	find_library(LibNL_LIBRARY NAMES libnl-3.a HINTS ${LibNL_ROOT_DIR}/lib/.libs)
	find_library(LibNL_ROUTE_LIBRARY NAMES libnl-route-3.a HINTS ${LibNL_ROOT_DIR}/lib/.libs)
	find_library(LibNL_NETFILTER_LIBRARY NAMES libnl-nf-3.a HINTS ${LibNL_ROOT_DIR}/lib/.libs)
	find_library(LibNL_GENL_LIBRARY NAMES libnl-genl-3.a HINTS ${LibNL_ROOT_DIR}/lib/.libs)
else ()
	find_library(LibNL_LIBRARY NAMES nl nl-3 HINTS ${LibNL_ROOT_DIR})
	find_library(LibNL_ROUTE_LIBRARY NAMES nl-route nl-route-3 HINTS ${LibNL_ROOT_DIR})
	find_library(LibNL_NETFILTER_LIBRARY NAMES nl-nf nl-nf-3 HINTS ${LibNL_ROOT_DIR})
	find_library(LibNL_GENL_LIBRARY NAMES nl-genl nl-genl-3 HINTS ${LibNL_ROOT_DIR})
endif ()

if (LibNL_INCLUDE_DIR AND LibNL_LIBRARY)
	set(LibNL_FOUND TRUE)
endif (LibNL_INCLUDE_DIR AND LibNL_LIBRARY)

if (LibNL_FOUND)
	set(LibNL_LIBRARIES ${LibNL_LIBRARY} ${LibNL_ROUTE_LIBRARY} ${LibNL_NETFILTER_LIBRARY} ${LibNL_GENL_LIBRARY})
        message("Found netlink libraries:  ${LibNL_LIBRARIES}")
        message("Found netlink includes: ${LibNL_INCLUDE_DIR}")
ELSE (LibNL_FOUND)
	if (LibNL_FIND_REQUIRED)
		message(FATAL_ERROR "Could not find netlink library.")
	endif (LibNL_FIND_REQUIRED)
endif (LibNL_FOUND)
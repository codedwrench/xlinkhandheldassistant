# - Try to find libpcap include dirs and libraries
#
# Usage of this module as follows:
#
#     find_package(PCAP)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  PCAP_ROOT_DIR             Set this variable to the root installation of
#                            libpcap if the module has problems finding the
#                            proper installation path.
#
# Variables defined by this module:
#
#  PCAP_FOUND                System has libpcap, include and library dirs found
#  PCAP_INCLUDE_DIR          The libpcap include directories.
#  PCAP_LIBRARY              The libpcap library (possibly includes a thread
#                            library e.g. required by pf_ring's libpcap)
#  HAVE_PF_RING              If a found version of libpcap supports PF_RING
#  HAVE_PCAP_IMMEDIATE_MODE  If the version of libpcap found supports immediate mode

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

find_path(PCAP_ROOT_DIR
    NAMES include/pcap.h
)

find_path(PCAP_INCLUDE_DIR
    NAMES pcap.h
    HINTS ${PCAP_ROOT_DIR}/include
)

set (HINT_DIR ${PCAP_ROOT_DIR}/lib)

# On x64 windows, we should look also for the .lib at /lib/x64/
# as this is the default path for the WinPcap developer's pack
if (${CMAKE_SIZEOF_VOID_P} EQUAL 8 AND WIN32)
    set (HINT_DIR ${PCAP_ROOT_DIR}/lib/x64/ ${HINT_DIR})
endif ()

if (NOT BUILD_STATIC)
    find_library(PCAP_LIBRARY
        NAMES pcap wpcap
        HINTS 
        ${CMAKE_CURRENT_SOURCE_DIR}/Objects
        ${PCAP_ROOT_DIR}
        ${HINT_DIR}
    )
else()
    if(MSVC)
        find_library(PCAP_LIBRARY
            NAMES libpcap.lib wpcap.lib
            HINTS 
            ${CMAKE_CURRENT_SOURCE_DIR}/Objects
            ${PCAP_ROOT_DIR}
            ${HINT_DIR}
        )
    else()
        find_library(PCAP_LIBRARY
            NAMES libpcap.a wpcap.a
            HINTS 
            ${CMAKE_CURRENT_SOURCE_DIR}/Objects
            ${PCAP_ROOT_DIR}
            ${HINT_DIR}
        )
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PCAP DEFAULT_MSG
    PCAP_LIBRARY
    PCAP_INCLUDE_DIR
)

include(CheckCXXSourceCompiles)
set(CMAKE_REQUIRED_LIBRARIES ${PCAP_LIBRARY})
check_cxx_source_compiles("int main() { return 0; }" PCAP_LINKS_SOLO)
set(CMAKE_REQUIRED_LIBRARIES)

# check if linking against libpcap also needs to link against a thread library
if (NOT PCAP_LINKS_SOLO)
    # We need threads in XLHA anyway so don't do the check here

    set(CMAKE_REQUIRED_LIBRARIES ${PCAP_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
    check_cxx_source_compiles("int main() { return 0; }" PCAP_NEEDS_THREADS)
    set(CMAKE_REQUIRED_LIBRARIES)

    if (PCAP_NEEDS_THREADS)
        set(_tmp ${PCAP_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
        list(REMOVE_DUPLICATES _tmp)
        set(PCAP_LIBRARY ${_tmp}
            CACHE STRING "Libraries needed to link against libpcap" FORCE)
    endif()

endif (NOT PCAP_LINKS_SOLO)

include(CheckFunctionExists)
set(CMAKE_REQUIRED_LIBRARIES ${PCAP_LIBRARY})
check_function_exists(pcap_get_pfring_id HAVE_PF_RING)
check_function_exists(pcap_set_immediate_mode HAVE_PCAP_IMMEDIATE_MODE)
check_function_exists(pcap_set_tstamp_precision HAVE_PCAP_TIMESTAMP_PRECISION)
set(CMAKE_REQUIRED_LIBRARIES)

mark_as_advanced(
    PCAP_ROOT_DIR
    PCAP_INCLUDE_DIR
    PCAP_LIBRARY
)

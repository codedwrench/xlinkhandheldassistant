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

# addcompilerflags.cmake - Adds compiler flags to the project.

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set global compile flags
#  https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_COMPILER_ID.html (for possible compiler ids)
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "/O1 /MT")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /MTd /Zi")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /MTd /ZI")
    set(PROJECT_COMPILE_WARNING_FLAGS /Wall /sdl /WX)
else()
    set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
    set(PROJECT_COMPILE_WARNING_FLAGS -Wall -Wextra -Wshadow -Wconversion -Weffc++)
    # Static libstdc++
    if (NOT APPLE)
        set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ ${CMAKE_EXE_LINKER_FLAGS}")
    else ()
        set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ ${CMAKE_EXE_LINKER_FLAGS}")
    endif()

endif()

if (MSYS OR MINGW)
    # Add WinSock, liphlpapi, wlanapi and ole32 to the platform specific libraries, MINGW needs it.
	set(CMAKE_CXX_STANDARD_LIBRARIES "-liphlpapi -lwlanapi -lole32 -lwinmm -lws2_32 ${CMAKE_CXX_STANDARD_LIBRARIES}")
    
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_CURRENT_SOURCE_DIR}/Resources/${CMAKE_PROJECT_NAME}.o -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive")
endif ()

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, defaulting to release.")
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "Building ${PROJECT_NAME} in ${CMAKE_BUILD_TYPE} mode")

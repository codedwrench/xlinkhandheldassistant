# MIT License
#
# Copyright © 2020 Rick de Bondt
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

# CMakeLists.txt - Main cmakelists file for this project

cmake_minimum_required(VERSION 3.15)
project(xlinkhandheldassistant DESCRIPTION "Helps XLink Kai bring handhelds online")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(AUTO_FORMAT "Automatically format code" ON)
option(BUILD_DOC "Build doxygen" OFF)
option(ENABLE_TESTS "Build unittests" OFF)
option(BUILD_STATIC "Statically link all libraries that can be statically linked" OFF)
option(BUILD_X32 "Cross compile the x32 variant for Windows" OFF)

# This is where CMake looks for submodules (including where to find libpcap and the like)
set(CMAKE_MODULE_PATH
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake"
	"${CMAKE_MODULE_PATH}"
	)

include(obtaingitversion)
include(addcompilerandlinkerflags)

# Check if Doxygen is installed
if (BUILD_DOC)
	include(adddoxygen)
endif ()

if (BUILD_STATIC)
	message("Static linking enabled")
	add_definitions(-DBUILD_STATIC)
	set(BUILD_STATIC ON)
endif ()

# Windows specific options
if (WIN32)
	set(BUILD_STATIC ON)

	if (MSYS OR MINGW)
		# MSYS specific steps (specifically for pcap and rc-files)
		
		# Add delay loaded library so npcap can be loaded properly
		execute_process(COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/build_scripts/generateDelayLoadedWPCap.bat"
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build_scripts
			)

		# Conversion from res file to .o file so we can link it in a way that will make the version info stick
		# See: https://stackoverflow.com/questions/63013037/mingw-winpthreads-overrides-rc-resource-file-information
		execute_process(COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/build_scripts/resourceToObject.bat"
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build_scripts
			)
	else()
		# MSVC specific steps
		list(APPEND PLATFORM_SPECIFIC_LIBRARIES "delayimp.lib")

		# We need to load wpcap
		add_link_options(/DELAYLOAD:wpcap.dll)

		# Add resource file as well
		list(APPEND ${CMAKE_PROJECT_NAME}_SOURCES PRIVATE Resources/${CMAKE_PROJECT_NAME}.rc)
	endif()
	
	# The logger module uses ERROR in an enum class, GDI also uses this, but screw GDI
	# We have no GUI anyway!
	add_definitions(-DNOGDI)

	set(PCAP_ROOT_DIR "c:/npcapsdk" CACHE STRING "Where the npcap sdk is located")
	set(PDCURSES_ROOT_DIR "c:/pdcurses/" CACHE STRING "Where the pdcurses directory is located")

elseif (UNIX)
	if (BUILD_STATIC)
		set(BOOST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../boost" CACHE STRING "Boost location")

		# PCAP and Curses already dealt with for Windows
		set(PCAP_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../libpcap/" CACHE STRING "LibPCAP location")
		set(NCURSES_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../ncurses/" CACHE STRING "NCurses location")

		if (NOT APPLE)
			set(LibNL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../libnl/" CACHE STRING "LibNL location")
		endif ()
	endif ()

	if (APPLE)
		# Use automatic reference counting for Objective-C portions
		add_compile_options(-fobjc-arc)

		find_library(CoreW CoreWLAN)
		list(APPEND PLATFORM_SPECIFIC_LIBRARIES ${CoreW})
		list(APPEND ${CMAKE_PROJECT_NAME}_SOURCES ${CoreW} ${CoreL})

		# HomeBrew installs libpcap here
		set(PCAP_ROOT_DIR "/usr/local/opt/libpcap")

		# Make it search for HomeBrew ncurses locations as well, the macOS version is not good enough
		set(NCURSES_ROOT_DIR "/usr/local/opt/ncurses")
	else ()

		# Linux uses LibNL to connect to adhoc networks
		find_package(LibNL REQUIRED)
		list(APPEND PLATFORM_SPECIFIC_LIBRARIES ${LibNL_LIBRARIES})
		list(APPEND PLATFORM_SPECIFIC_INCLUDES ${LibNL_INCLUDE_DIR})
	endif ()
endif()


if (BUILD_STATIC)
	set(BOOST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../boost" CACHE STRING "Boost location")

	# PCAP and Curses already dealt with for Windows
	if (NOT WIN32)
		set(PCAP_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../libpcap/" CACHE STRING "LibPCAP location")
		set(NCURSES_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../ncurses/" CACHE STRING "NCurses location")

		if (UNIX AND NOT APPLE)
			set(LibNL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../libnl/" CACHE STRING "LibNL location")
		endif()
	endif()
endif()

if (WIN32)
	find_package(PDCurses REQUIRED)

else ()
	set(CURSES_NEED_WIDE TRUE)
	set(CURSES_NEED_NCURSES TRUE)
	find_package(PCAP REQUIRED)
	find_package(Curses REQUIRED)
endif ()

include(wrapfindboost)
include(wrapfindthreads)

set(TARGET_NAME ${CMAKE_PROJECT_NAME})
set(${TARGET_NAME}_SOURCES main.cpp)

add_subdirectory(Includes)
add_subdirectory(Sources)

# Targets
list(APPEND targets ${CMAKE_PROJECT_NAME})
list(APPEND ${CMAKE_PROJECT_NAME}_INCLUDE_FOLDERS ${PCAP_INCLUDE_DIR} ${Boost_INCLUDE_DIR} ${CURSES_INCLUDE_DIRS} ${PLATFORM_SPECIFIC_INCLUDES})
list(APPEND ${CMAKE_PROJECT_NAME}_LIBRARIES Threads::Threads ${PCAP_LIBRARY} ${Boost_LIBRARIES} ${CURSES_LIBRARIES} ${PLATFORM_SPECIFIC_LIBRARIES})

if (ENABLE_TESTS)
	# Same libraries available to the unittests as the main program
	add_subdirectory(Tests)

	find_package(GTest REQUIRED)
	include(GoogleTest)
	enable_testing()
endif(ENABLE_TESTS)

include(addtargets)

# Generate manual PDF
if(BUILD_DOC)
	add_custom_target(build_setupguide ALL
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Docs
			COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/Docs/generate_pdf.sh"
			COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/Docs/README.pdf ${CMAKE_CURRENT_BINARY_DIR}
			VERBATIM)
endif()

# Copying files to release dir
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Sources/UserInterface/Themes
	DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Plugin
	DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

if(UNIX AND NOT APPLE)
	# Some linux specific files
	file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/linux_scripts
		DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

	file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Resources/icon.svg
		DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Resources)

	file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Resources/${CMAKE_PROJECT_NAME}.desktop
		DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Resources)

elseif (APPLE)
	# Apple specific files
	file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/entitlements.plist
			DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()


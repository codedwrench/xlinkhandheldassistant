## Linux
### Debian Testing and above
This program has only been tested on Debian Testing and above.
It requires the following packages to be installed:
- cmake
- gcc (version 10 or higher)
- g++ (version 10 or higher)
- libboost-dev (version 1.71 or above)
- libboost-program-options-dev
- libncurses-dev
- libpcap-dev
- libpthread-stubs0-dev
- libnl-3-dev
- libnl-genl-3-dev
- libnl-nf-3-dev
- libnl-route-3-dev
- libpthread-stubs0-dev

### Arch Linux
- libnl
- cmake
- boost
- boost-libs
- libpcap
- ncurses
- ncurses5-compat-libs
- gcc
- libpthread-stubs

If those packages are installed, you can compile the program using the following command (from the project's root):
```bash
mkdir build && cd build && cmake .. && cmake --build . -- -j`nproc`
``` 

## Windows
Releases on Windows are compiled using MSYS2. Visual Studio should also work, but it might need some small fixing.

**Note:** Monitor mode is not available on Windows. NPcap does not support packet injection, see: https://github.com/nmap/npcap/issues/85 .

The following programs are needed:
- MSYS2 with a GCC version of atleast 10. (Visual Studio 2019 or higher is also possible)  
  Installed in MSYS2:
  - bison
  - mingw-w64-i686-cmake
  - mingw-w64-i686-gcc
  - mingw-w64-i686-make
  - mingw-w64-i686-tools-git

- CMake, if using Visual Studio 2019/2022, this is built into it.

The following libraries are needed:
- Boost 0.7.1 or higher https://www.boost.org/users/download/ (system and program_options required 
  (Visual Studio also wants date-time for some reason)).  
  Commands used (from root of boost dir (in MSYS2)): 
  - cd tools/build
  - ./bootstrap.bat mingw
  - cp b2 ../..
  - cd ../..
  - ./tools/build/b2.exe --build-type=minimal --toolset=gcc variant=release link=static runtime-link=static
    address-model=32 threading=multi --with-system --with-program_options stage
- PDCurses or other ncurses compatible library (only pdcurses tested) https://github.com/Bill-Gray/PDCursesMod
  Commands used (from root of pdcurses dir)
  - cd wincon
  - mingw32-make
- NPcap and NPcap SDK \
  https://nmap.org/npcap/dist/npcap-1.60.exe \
  https://nmap.org/npcap/dist/npcap-sdk-1.06.zip

After installing these, open the CMakeLists.txt and set the paths to the libraries to the paths you installed these libraries to. \
By default it looks in the following paths:
- c:\boost\
- c:\pdcurses\
- c:\npcapsdk

If that is all in order you should be able to compile the program using Visual Studio 2019 or higher, by opening the project using it and then pressing the compile button.

For MSYS2 you should be able to run the following commands:
```batch
mkdir build 
cd build 
cmake .. -G "MinGW Makefiles"
mingw32-make
``` 

## MacOS
For MacOS you need Brew and XCode.
You need the following packages:
- boost
- ncurses
- libpcap
- cmake

If those packages are installed, you can compile the program using the following command (from the project's root):
```bash
mkdir build && cd build && cmake .. -G"XCode" && cmake --build . -- -j`nproc`
``` 

Then you can optionally sign the executable with the following command  
(**Note:** you need to be in the Apple Developer
Program for this or have AMFI disabled).

```bash
codesign --force --sign --deep --entitlements ./entitlements.plist -s "NAME-OF-CERT" ./xlinkhandheldassistant
```

And to check if that command did its job:

```bash
codesign -d --entitlements - ./xlinkhandheldassistant
```

After that the program should be able to run.


## Building Statically
For Linux and MacOS a static build can be done by adding the following to the cmake command:  
```-DBUILD_STATIC=1```

The libraries should be statically compiled and put in the parent folder above XLHA.
Then compiling should be done in the same way as described above for your Operating System.

The Windows version is always statically compiled!
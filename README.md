# XLHA - XLink Handheld Assistant
Helps handheld devices connect to XLink Kai using a variety of methods.

At this point receive and send PSP traffic succesfully through XLink Kai using the following methods:
- Monitor mode
- Plugin mode

## Wifi cards that have been tested and found working on Monitor Mode (Linux, kernel 5.x):

### Atheros
- Azurewave AR5BHB92 (Atheros 9280 (ath9k))
- TP-Link TL-WDN4800 (Atheros AR93xx (ath9k))
- NetGear, Inc. WNA1100 Wireless-N 150 (Atheros 9271 (ath9k_htc))
- TP-Link TL-WN821N v3 (Atheros 9287 (ath9k_htc))
- Qualcomm Atheros QCA6164 802.11ac Wireless Network Adapter (ath10k_pci)

### Broadcom
- Dell Wireless 1390 (Broadcom BCM4311 (b43)) with the following patch: https://gist.github.com/codedwrench/8d7916d63993574e1dd089a62dd523a9

### Intel
- Intel Centrino Wireless-N 1000 (iwlwifi)
- Intel Centrino Advanced-N 6205 (iwlwifi)
- Intel Pro Wireless 5100 AGN (iwlwifi)
- Intel® Wi-Fi 6E AX210 (Gig +) (iwlwifi)

### Ralink
- TP-Link TL-WN727N v1 (Ralink rt3070 (rt2800usb))
- WTXUP Ralink RT3070 (rt2800usb)
- TOOGOO®/SODIAL® Mini 150Mbps USB WiFi Wireless LAN 802.11 n/g/b Netzwerk Adapter Ralink RT5370 (rt2800usb)
- Planex GW-US54Mini2 (Ralink rt2501usb (rt73))
- Linksys WUSB600N v1 Dual-Band Wireless-N Network Adapter Ralink RT2870 (rt2800usb)

### Realtek
- Hercules Guillemot HWGUm-54 (rtl8192su (codedwrench/rtl8192su))
- TP-Link TL-WN823N V2 (Realtek rtl8192eu (kimocoder/rtl8192eu))

## Wifi cards that have been tested and found working on Plugin Mode (Linux, kernel 5.x / Windows 10):
### Note:
On Windows it has been found that some drivers will not allow for selecting a channel when forming an adhoc network, this is only an issue with SSID swapping games when "Use SSID from host broadcast" is used.

### Atheros
- TP-Link TL-WDN4800 (Atheros AR93xx (ath9k)) 
  (On Windows 10 some people are seeing some problems, this issue is not there on Linux) (Needs Windows 7 drivers on Windows 10)
- NetGear, Inc. WNA1100 Wireless-N 150 (Atheros 9271 (ath9k_htc))
  (On Windows 10 some people are seeing some problems, this issue is not there on Linux) (Needs Windows 7 drivers on Windows 10)

### Intel
- Intel Pro Wireless 5100 AGN (iwlwifi)
  (Needs Windows 7 drivers on Windows 10)
  
### Ralink
- WTXUP Ralink RT3070 (Ralink rt2800usb)

### Realtek
- Asus USB-N13 rev b. (Realtek rtl8192cu)
- Hercules Guillemot HWGUm-54 (Realtek rtl8192su (linux: codedwrench/rtl8192su))
- TP-Link TL-WN823N V2 (Realtek rtl8192eu (linux: kimocoder/rtl8192eu))

## How to compile

## Debian Testing and above
This program has only been tested on Debian Testing and above.
It requires the following packages to be installed:
- cmake
- gcc-10
- g++-10
- libboost-dev (version 1.71 or above)
- libboost-program-options-dev
- libncurses5-dev
- libpcap-dev
- libpthread-stubs0-dev
- libnl-3-dev
- libnl-genl-3-dev
- libnl-nf-3-dev

## Arch Linux
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
This program occasionally gets compiled for Windows 10 using Visual Studio 2019. MINGW64 with a GCC version of atleast 10 works as well.

**Note:** Monitor mode is not available on Windows. NPcap does not support packet injection, see: https://github.com/nmap/npcap/issues/85 .

The following programs are needed:
- Visual Studio 2019 or MINGW64 with a GCC version of atleast 10
- CMake, if using Visual Studio 2019, this is built into it

The following libraries are needed:
- Boost 0.7.1 or higher https://www.boost.org/users/download/ (threads and program_options required)
- PDCurses or other ncurses compatible library (only pdcurses tested) https://github.com/Bill-Gray/PDCursesMod \
  (Make sure you use the x64 toolchain to compile this, otherwise you'll get very undescriptive errors when trying to compile the program)
- NPcap 0.9982 and NPcap SDK \
  https://nmap.org/npcap/dist/npcap-0.9982.exe \
  https://nmap.org/npcap/dist/npcap-sdk-1.06.zip
  
After installing these, open the CMakeLists.txt and set the paths to the libraries to the paths you installed these libraries to. \
By default it looks in the following paths:
- c:\Program Files\boost\boost_1_71_0\output
- c:\pdcurses\
- c:\npcapsdk

If that is all in order you should be able to compile the program using Visual Studio 2019 or higher, by opening the project using it and then pressing the compile button. 

For MINGW64 you should be able to run the following commands:
```batch
mkdir build 
cd build 
cmake .. -G "MinGW Makefiles"
mingw32-make
``` 

After compiling, the program needs the following DLLs to be copied over to the binary directory:
- boost_program_options-(compiler)-(architecture)-(version).dll
- Packet.dll (found in: c:\windows\system32\npcap\)
- wpcap.dll (found in: c:\windows\system32\npcap\)

After that the program should be able to run.


## How to run 
See [Guide](Docs/README.md)

## Known issues
- Monitor mode on Windows does not work.
- Plugin mode fails to work on some Atheros cards without a custom plugin on Windows 10.
- Packet injection fails when the packet is at an MTU of 1500 (max size), solved by setting a higher MTU in the WiFi card.

## Contributing
See [Contributing](CONTRIBUTING.md)

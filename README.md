# XLHA - XLink Handheld Assistant
Helps handheld devices connect to XLink Kai using a variety of methods.

[![Build Status](https://github.com/codedwrench/xlinkhandheldassistant/actions/workflows/build.yml/badge.svg)](https://github.com/codedwrench/xlinkhandheldassistant/actions)
[![Release](https://img.shields.io/github/release/codedwrench/xlinkhandheldassistant.svg)](https://github.com/codedwrench/xlinkhandheldassistant/release)
[![Discord](https://img.shields.io/badge/Discord-XLink%20Kai-brightgreen)](https://discord.gg/XUS9n73KSP)

At this point receive and send PSP traffic succesfully through XLink Kai using the following methods:
- Monitor mode
- Plugin mode
- Vita Mode

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

### Mediatek
- Xbox Wireless Adapter for Windows (mt76x2u) [Adapter](https://www.amazon.com/Xbox-One-Wireless-Adapter-Windows/dp/B00ZB7W4QU/)

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


### Broacom
- Dell DW1397 (BCM43XNG20) (Windows driver: [here](https://cdn.teamxlink.co.uk/drivers/Broadcom_BCM43_Drivers_only_2022.03.22.7z))

### Intel
- Intel Pro Wireless 5100 AGN (iwlwifi)
  (Needs Windows 7 drivers on Windows 10)
- Intel Centrino Advanced-N 6205 (iwlwifi)
  
### Ralink
- WTXUP Ralink RT3070 (Ralink rt2800usb)

### Realtek
- Asus USB-N13 rev b. (Realtek rtl8192cu)
- Hercules Guillemot HWGUm-54 (Realtek rtl8192su (linux: codedwrench/rtl8192su))
- TP-Link TL-WN823N V2 (Realtek rtl8192eu (linux: kimocoder/rtl8192eu))
- NETIS USB (Realtek rtl8188eu) (Needs Windows 7 drivers on Windows 10)

## How to compile
See [Compiling](COMPILING.md)

## How to run 
See [Guide](Docs/README.md)

## Known issues
- Monitor mode on Windows does not work.
- Plugin mode fails to work on some Atheros cards without a custom plugin on Windows 10.
- Packet injection fails when the packet is at an MTU of 1500 (max size), solved by setting a higher MTU in the WiFi card.

## Contributing
See [Contributing](CONTRIBUTING.md)

---
author: Rick de Bondt
fontfamily: dejavu
fontsize: 8pt
geometry: margin=3cm
urlcolor: blue
output: pdf_document
---
# XLink Handheld Assistant - How to use
For XLink Handheld Assistant there are a few modes the assistant can run in, namely:

- Monitor mode (only on Linux)
- Plugin mode (only for PSP)

These modes have their own upsides and downsides which will be explained below.

- Monitor mode: can only be used on Linux and is generally less reliable than the Plugin mode, its reliability highly depends on the WiFi card used and how busy the WiFi space is in general. The upside of Monitor mode is that it also works on Playstation Vita games, and hopefully in the future more devices as well.
- Plugin mode: can be used on both Windows and Linux, however it only works on PSP games (PSP or Vita using Adrenaline) and needs a plugin to be enabled on the respective device, so the device needs to be modded (CFW/HEN) as well.

\
Follow one of the following guides depending on your chosen method and operating system:

- [Linux - Plugin Mode][]

\newpage
## Linux - Plugin Mode
1. Copy [AdHocRedirectorWiFi.prx](./Plugin/AdHocRedirectorWiFi.prx) to /SEPLUGINS on the PSP.
2. Create or open /SEPLUGINS/GAME.
3. Add 

   ```ms0:/seplugins/AdHocRedirectorWiFi.prx 1``` (for PSP 1000/2000/3000) 

   or 

   ``` ef0:/seplugins/AdHocRedirectorWiFi.prx 1``` (for PSP GO)
\
   ```bash
   touch /media/username/nameofPSP/SEPLUGINS/GAME.TXT \
   && echo "ms0:/seplugins/AdHocRedirectorWiFi.prx 1" \
   >> /media/username/nameofPSP/SEPLUGINS/GAME.TXT
   ```
   
4. Start a game on the PSP.
5. Run the following to set the wifi interface to ad-hoc mode. \
\
**NOTE:** This does disable NetworkManager, if another WiFi connection is used to connect to the internet, it might be better to make NetworkManager ignore the WiFi adapter used for XLHA and edit the "set_interface_bss" script to not disable NetworkManager. \

    ```bash
    sudo chmod 775 ./linux_scripts/set_interface_bss.sh \
    && ./linux_scripts/set_interface_bss.sh "Name of Wifi adapter"
    ```

6. Start [XLink Kai](http://teamxlink.co.uk/) on the PC
    1. Follow steps on [XLink Kai Debian Guide](https://repo.teamxlink.co.uk/) or download the binary from the [download page](https://www.teamxlink.co.uk/go?c=download), after downloading: 
    
       ```bash
       tar xvf kaiEngine-*.tar.gz \
       && cd "$(ls -f | grep kaiEngine | grep -v *.tar.gz)" \
       && sudo chmod 755 ./kaiengine \
       && sudo setcap cap_net_admin,cap_net_raw=eip ./kaiengine \
       && ./kaiengine
       ```
7. Start XLink Handheld Assistant 
   ```bash
   sudo chmod 755 ./xlinkhandheldassistant \
   && sudo setcap cap_net_admin,cap_net_raw=eip ./xlinkhandheldassistant \
   && ./xlinkhandheldassistant
   ```
   If you get an error about terminal size at this point, make sure your terminal is atleast of size 80x24.
   Example: Terminal size too small: 46x18, make sure the terminal is at least 80x24
   
8. In the wizard select "Plugin Device" and then use the arrow keys to move down to the "Next" button and press enter.
9. Press space on "Automatically connect to PSP networks".
10. Move down to the right network adapter with the arrow keys and press space on the WiFi adapter to be used for XLHA.
11. Move to "Next" button and press enter.
12. Press enter on the "Next" button again.
13. In the Dashboard if you're hosting press space on "Hosting", else just move to the next step.
14. Press enter on "Start Engine".
15. Enter the arena on XLink Kai you want to play on [WebUI](http://127.0.0.1:34522/)
16. On the PSP side go into the multiplayer menu start hosting/joining a game.
17. In the top right-corner of the Dashboard "Connected to PSP_GameID_...." should appear after 5-30 seconds.
18. Enjoy the game!
19. To stop XLHA, simply press 'q' in the Dashboard.
20. Run the following to restore your WiFi-card to the normal situation:
    ```bash
    sudo chmod 775 ./linux_scripts/restore_managed.sh \
    && ./linux_scripts/restore_managed.sh "Name of Wifi adapter"
    ```

\
If you want to redo these steps or choose another connection method, go into "Options" -> "Reconfigure the application"

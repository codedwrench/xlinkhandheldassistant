# Helper Scripts
The scripts in this folder help you set up your WiFi card for use with XLink Handheld Assistant.
These scripts assume you are normally connected by wire! If this is not the case you may need to follow different steps.
This is because these scripts turn NetworkManager off!

The following scripts are available:
- set_interface_bss.sh
- set_interface_monitor.sh
- restore_managed.sh
- start-xlinkhandheldassistant-cli.sh
- start-xlinkhandheldassistant.sh
- dissect_log_psp_side.py
- dissect_log_xlink_side.py

----
- set_interface_bss.sh is used for PSP plugin mode.
- set_interface_monitor.sh is used for monitor mode.
- restore_managed.sh is used to restore the default mode and start networkmanager back up.
- dissect_log_psp_side.py is used to grab raw packets from a log file in TRACE mode.
- dissect_log_xlink_side.py is used to grab raw packets from a log file in TRACE mode.
- start-xlinkhandheldassistant-cli.sh tries to start xlinkhandheldassistant in one go setting the wifi adapter to its 
  correct mode immediately.
- start-xlinkhandheldassistant.sh calls start-xlinkhandheldassistant-cli.sh using a x-terminal-emulator (change to your own terminal).

## Starting XLHA using a .desktop shortcut
- Edit start-xlinkhandheldassistant.sh and start-xlinkhandheldassistant-cli.sh to fit your own environment.
- Copy Resources/XLHA.desktop to /usr/share/applications editing the path to wherever xlinkhandheldassistant is located.

# Editor connection fix — RED MASS VAMPIRE OS v0.5.1-alpha

The Full editor could show **short reply** while connecting to a device running FM-1_906. Its VAMPIRE engine reports 99 factory presets in a single SysEx reply, but that reply reaches the firmware's 600-byte output limit before all the names are sent.

This application update recognizes that specific firmware version and validates every complete received name against its bundled factory catalog. When the prefix matches, it supplies the remaining names from the same catalog and completes the connection. A reply from a different firmware or a mismatched catalog is rejected rather than silently displaying incorrect names.

**Install the app update:** extract the whole updated ZIP into a fresh folder. Fully exit the earlier RED MASS VAMPIRE OS app from the tray, launch `RED-MASS-VAMPIRE-OS.exe` from the new folder, open **Full editor** and connect again. The firmware package and update identity are unchanged; this editor fix does **not** require reflashing the FM-1.

Host validation compares the editor with the native C firmware's captured 600-byte reply, checks all 99 names, and verifies that a mismatched reply is rejected. Physical USB-MIDI testing on an FM-1 is still pending.

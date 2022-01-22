PS2 HL RAM FS tool v1.00

Developed by supadupaplex, 2022
License: BSD-3-Clause (check out license.txt)
Zlib library is used to perform deflate operations

This tool can extract RAM FS contents from PCSX2 save state or EE RAM dump,
it is intended for direct save dictionary harvesting (*.hl1/2/3 files)
for PAKS/DICTS.PAK.

(!) Versions supported so far: SLUS_200.66 and SLES_505.04\n\

How to prepare PCSX2 save state for desired map:
1) Start the game in PCSX2
2) Go to a desired map
3) Do a quick save in the pause menu (Pause menu -> Quick Save)
4) Perform a save state in the PCSX2 (System -> Save state -> Slot X)

How to use:
1) Windows explorer - drag and drop PCSX2 save state file on rfstool.exe
2) Command line/Batch - rfstool [file_name]

For more info check out readme.txt

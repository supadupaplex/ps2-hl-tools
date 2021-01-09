PS2 HL sprite tool
Developed by supadupaplex
License: BSD-3-Clause (check out license.txt)

This tool is intended to convert PS2 Half-Life *.SPZ sprites to PC's *.SPR and vice versa.

PS2 HL *.SPZ sprites can have frames with fixed dimensions: 8 (min), 16, 32, 64, 128, 256, ... .
Game graphics may be corrupted during playback of sprites with inappropriate dimensions.
Frames with inappropriate sizes would be automatically resized during conversion to *.SPZ format.

In case of conversion to *.SPR format you have an option to resize frames back to their
original sizes or to keep them as is to avoid resizing artifacts.

Because *.SPZ sprites lack some parameters that present in *.SPR, so these parameters are set to:
1) BeamLength = 0
2) SyncType = 1 (RANDOM)

How to use:
1) Windows explorer - drag and drop sprite file on sprtool.exe
2) Command line\Batch - sprtool (noresize/lin) [file_name]

Changelog:
v1.3 - experimental linear resize

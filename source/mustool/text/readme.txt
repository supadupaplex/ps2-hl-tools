PS2 HL music tool
Developed by supadupaplex
License: BSD-3-Clause (check out license.txt)

This tool is intended to patch PS2 Half-Life headerless *.VAG music
files so then they can be converted with Awave studio or Video Game
Sound Converter (VGSC). This tool also can perform reverse process
to make *.VAG music files usable in PS2 Half-life but remember that
PS2 HL supports only 44100 Hz mono audio VAGs.

Changelog:
- v1.1: initial support for PS2 HL *.WAV files with compressed headers
(those are inside compressed PAKs). If you want to make your own compressed
PS2 HL *.WAV remember that it should be 8-bit 11025/22050/44100 Hz mono audio.
- v1.2: proper support for PS2 HL *.WAV files with compressed headers
- v1.23: added support for looped WAVs

How to use:
1) Windows explorer - drag and drop *.VAG\*.WAV audio file on mustool.exe
2) Command line\Batch:
	I) Make *.VAG\*.WAV audio file readable for Awave\VGSC:
		mustool patch [filename]
	II) Make *.VAG\*.WAV audio file readable for PS2 HL:
		mustool unpatch [filename]
	III) Check *.VAG\*.WAV audio file:
		mustool test [filename]

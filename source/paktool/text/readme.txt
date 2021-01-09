PS2 HL PAK tool
Developed by supadupaplex
License: BSD-3-Clause (check out license.txt)
Zlib library is used within this program to perform deflate\inflate operations

This tool is intended to extract or create new PS2 Half-life PAK files.

Changelog:
- v1.22: I found out that address of GLOBAL.PAK deviates, so I added possibility to enter it manually
- v1.3:  I fixed address calculation for GLOBAL.PAK so there is no more need to manually enter it
- v1.31: Added support for 16-byte aligned non-compressed PAKs

How to use:
1) Windows explorer - drag and drop file or directory on paktool.exe

2) Command line\Batch:
	paktool (option) [file\dir_name]
	
	List of options:
	- test			- print info about PAK file
	- extract		- extract PAK
	- pack			- pack files from specified directory to normal PAK
	- pack16		- pack files from specified directory to normal PAK with 16-byte alignment (i.e. pausegui.pak)
	- cpack			- pack files from specified directory to compressed PAK
	- gpack			- pack files from specified directory to GLOBAL.PAK and GRESTORE.PAK
	- decompress	- decompress PAK
	- compress		- compress PAK

Prefixes of generated files and folders:
1) "cmp-" - compressed file
2) "dec-" - decompressed file
3) "gre-" - patched (GRESTORE) file
4) "ext-" - folder with extracted files

Additional feature: you can decompress Zlib files (if you open them in hex editor you can find bytes 0x78, 0xDA).
Create new file with four 0x01 bytes, then paste compressed data (starting from 0x78DA) and then use "decompress" command.

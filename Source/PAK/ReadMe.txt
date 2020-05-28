PS2 HL PAK tool
Developed by Alexey Leushin, Novosibirsk, 2017-2020
Zlib library is used within this program to perform DEFLATE\INFLATE operations.

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

And finally, some formal stuff:
=====================================================================
LICENSE
=====================================================================
Copyright (c) 2017-2020, Alexey Leushin
All rights reserved.

Redistribution and use in source and binary forms, with or
without modification, are permitted provided that the following
conditions are met:

- Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

- Neither the name of the copyright holders nor the names of its
contributors may be used to endorse or promote products derived
from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
=====================================================================
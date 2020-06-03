PS2 HL decal (PHD) tool
Developed by Alexey Leusin, Novosibirsk, 2017
Zlib library is used within this program to perform DEFLATE\INFLATE operations.

This tool is intended to convert PS2 Half-Life decals to PNG images and vice versa.

v1.22 Initial support for BMP decals, PHD to BMP conversion is not 100% accurate.
v1.25 Added alternative PHD to BMP conversion mode which is supposed to be more accurate (hacky).

How to use:
1) Windows explorer - drag and drop decal file on phdtool.exe
2) Command line\Batch - phdtool (option) [file_name]
Options:
	topng	- PHD to PNG conversion
	tobmp	- PHD to BMP conversion
	altbmp	- PHD to BMP conversion with palette tweak (closer to original PC decals)
	hq	- BMP\PNG to PHD conversion with higher resolution (experimental, tanks RAM)
		  and palette tweak


And finally, some formal stuff:
=====================================================================
LICENSE
=====================================================================
Copyright (c) 2017, Alexey Leushin
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
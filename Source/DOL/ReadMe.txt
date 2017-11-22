PS2 HL model tool
Developed by Alexey Leusin, Novosibirsk, 2017

This tool is intended to convert PS2 Half-Life *.DOL models to PC's *.MDL format and vice versa.

How to use:
1) Windows explorer - drag and drop model file on mdltool.exe
2) Command line\Batch:
		mdltool [filename]
	Optional feature - extract textures from model:
		mdltool extract [filename]

If you want to change warning behaviour then edit "settings.ini".

Note, that PS2 Half-life models should have textures with dimensions of fixed sizes:
16 (min), 32, 64, 128, 256, ... . Otherwise either game crashes or graphics become corrupted.
In case of MDL do DOL conversion this program would automatically resize textures with wrong dimensions.

Also note, that some models like barney, scientist and hgrunt can’t be directly converted from PS2 to PC.
You need to decompile these models and remove LOD submeshes in *.qc file. Otherwise you may encounter
some glitches: barney shooting without pistol, all scientists have Walter (Kleiner) heads, all soldiers
are wearing gas masks.

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
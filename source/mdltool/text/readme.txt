PS2 HL model tool
Developed by Alexey Leusin, Novosibirsk, 2017-2018

This tool is intended to convert PS2 Half-Life *.DOL models to PC's *.MDL format and vice versa.

Changelog:
- v1.09: added support for *.PVR texture extraction from Dreamcast HL models,
	 added restriction on converting Dreamcast *.MDL files
- v1.11: removed settings file, extra data from *.DOL models is saved now to external *.INF files,
	 those *.INF files can be used during conversion from *.MDL back to *.DOL format

How to use:
1) Windows explorer - drag and drop model file on mdltool.exe
2) Command line\Batch:
		mdltool [filename]
	Optional features:
		- extract textures from model:
			mdltool extract [filename]
		- report sequences:
			mdltool seqrep [filename]

Note, that PS2 Half-life models should have textures with power of 2 dimensions:
8 (min), 16, 32, 64, 128, 256, ... . Otherwise either game crashes or graphics become corrupted.
In case of *.MDL do *.DOL conversion this program would automatically resize textures with wrong dimensions.

Also keep attention on *.INF files when converting from *.DOL to *.MDL. If you see LOD table inside
then consider to recompile corresponding model and remove LOD submeshes from it (in *.qc file).
Otherwise you may encounter some glitches while using converted models in PC Half-Life: barney shooting
without pistol, all scientists have Walter (Kleiner) heads, all soldiers are wearing gas masks.

Also keep in mind that some LOD tables are really messed up for some reason (scientist.dol for example) so
you might need to fix them in *.INF file by hand to properly reconvert back to *.DOL format.

Syntax of *.INF files:

	// Comment
	parameter[value]
	
	group
	{
	part1[LOD1_Distance,LOD2_Distance,LOD3_Distance,LOD4_Distance]
	part2
	part3[LOD1_Distance]
	}

- Please, don't insert any spaces or tabs in any line
- Number of distances inside [] should correspond to a number of LODs (4 is the max)
- If body part doesn't have LODs then don't put [] at all on its line (like part2 on example)

Available parameters:
fadestart	- distance at which model starts to fade
fadeend		- distance at which model completely fades away
groups		- number of body groups
maxparts	- maximum number of body parts inside one body group

And finally, some formal stuff:
=====================================================================
LICENSE
=====================================================================
Copyright (c) 2017-2018, Alexey Leushin
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
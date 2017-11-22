PS2 HL sprite tool
Developed by Alexey Leusin, Novosibirsk, 2017

This tool is intended to convert PS2 Half-Life *.SPZ sprites to PC's *.SPR and vice versa.

PS2 HL *.SPZ sprites can have frames with fixed dimensions: 16 (min), 32, 64, 128, 256, ... .
Game graphics may be corrupted during playback of sprites with inappropriate dimensions.
Frames with inappropriate sizes would be automatically resized during conversion to *.SPZ format.

In case of conversion to *.SPR format you have an option to resize frames back to their
original sizes or to keep them as is to avoid resizing artifacts.

Because *.SPZ sprites lack some parameters that present in *.SPR, so these parameters are set to:
1) BeamLength = 0
2) SyncType = 1 (RANDOM)

How to use:
1) Windows explorer - drag and drop sprite file on sprtool.exe
2) Command line\Batch - sprtool (option) [file_name]
Options:
	- noresize	- do not resize sprites during SPZ to SPR conversion
	- hq		- SPR to SPZ conversion with higher resolution (experimental, tanks RAM)

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
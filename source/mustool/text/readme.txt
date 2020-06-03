PS2 HL music tool
Developed by Alexey Leusin, Novosibirsk, 2017-2018

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
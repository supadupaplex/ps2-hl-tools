PS2 HL EPC tool
Developed by Alexey Leushin, Novosibirsk, 2018.

This tool is intended to build PS2 Half-life *.epc precache files.
If you see debug messages like "Loading sequence models\barney01.dol"
then you can fix them with this tool and thus reduce stuttering.

How to use:
1) Windows explorer - drag and drop *.txt file on epctool.exe
2) Command line\Batch - epctool.exe [file_name]

Formatting:
MAP_NAME1
{
MODEL_NAME1[SUBMODEL_NUM1,...,SUBMODEL_NUMn]
...
MODEL_NAMEn[SUBMODEL_NUM1,...,SUBMODEL_NUMn]
}

MAP_NAME2
{
MODEL_NAME1[SUBMODEL_NUM1,...,SUBMODEL_NUMn]
...
MODEL_NAMEn[SUBMODEL_NUM1,...,SUBMODEL_NUMn]
}

...

Rules:
- do not put one map two times on the list
- do not put maps with empty model list
- do not put one model two times inside one map list
- do not put lines or enters between MAP_NAME and {
- { and } should be on separate lines
- do not put any spaces or tabs in MODEL_NAME line
- adding exclamation mark at the end of the MODEL_NAME line
  sets up the flag that I found set only in deathmatch maps
- I don't know if precaching can be used for anyting else
  than models (never seen that in original files)

Example:
c0a0
{
models/barney.dol[1]
models/scientist.dol[2,3,5]
models/w_9mmar.dol[]
}

And finally, some formal stuff:
=====================================================================
LICENSE
=====================================================================
Copyright (c) 2018, Alexey Leushin
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
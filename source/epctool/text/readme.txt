PS2 HL EPC tool
Developed by supadupaplex
License: BSD-3-Clause (check out license.txt)

This tool is intended to build PS2 Half-life *.epc precache files.
If you see debug messages like "Loading sequence models\barney01.dol"
then you can fix them with this tool and thus reduce stuttering.
You can obtain animation sequence source file by playing converted mod
in "YA PS2HL" with "sv_ps2_precache" and "sv_supadupaplex" cvars set to 1 or 2.

How to use:
1) Windows explorer - drag and drop *.txt file (list) or *.inf file (source) on epctool.exe
2) Command line\Batch - epctool [file_name]

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

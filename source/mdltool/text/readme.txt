PS2 HL model tool
Developed by supadupaplex
License: BSD-3-Clause (check out license.txt)

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

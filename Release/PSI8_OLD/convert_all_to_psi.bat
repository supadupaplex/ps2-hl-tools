@echo off

echo Converting all *.bmp images in current folder ...

echo Note that bmps must be 8-bit palettized ones

echo __________________________________________________

for %%I in (*.bmp) do psitool.exe  %%I

echo __________________________________________________

echo Done. Press any key to exit ...

pause
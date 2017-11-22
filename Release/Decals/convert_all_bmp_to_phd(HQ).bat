@echo off

echo Converting all *.bmp images in current folder (HQ) ...

echo __________________________________________________

for %%I in (*.bmp) do phdtool.exe hq %%I

echo __________________________________________________

echo Done. Press any key to exit ...

pause

@echo off

echo Converting all *.png images in current folder ...

echo __________________________________________________

for %%I in (*.png) do phdtool.exe %%I

echo __________________________________________________

echo Done. Press any key to exit ...

pause

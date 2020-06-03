@echo off

echo Converting all *.png images in current folder ...

echo __________________________________________________

for %%I in (*.png) do psitool.exe  %%I

echo __________________________________________________

echo Done. Press any key to exit ...

pause
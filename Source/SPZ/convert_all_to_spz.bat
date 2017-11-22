@echo off

echo Converting all *.spr sprites in current folder ...

echo __________________________________________________

for %%I in (*.spr) do sprtool.exe %%I

echo __________________________________________________

echo Done. Press any key to exit ...

pause
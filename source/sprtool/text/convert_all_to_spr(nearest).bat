@echo off

echo Converting all *.spz sprites in current folder ...

echo __________________________________________________

for %%I in (*.spz) do sprtool.exe %%I

echo __________________________________________________

echo Done. Press any key to exit ...

pause
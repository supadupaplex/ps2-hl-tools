@echo off

echo Converting all *.spz sprites in current folder (no resize) ...

echo __________________________________________________

for %%I in (*.spz) do sprtool.exe noresize %%I

echo __________________________________________________

echo Done. Press any key to exit ...

pause
@echo off

echo Patching all *.VAG music files in current folder ...

echo __________________________________________________

for %%I in (*.VAG) do mustool.exe patch %%I

echo __________________________________________________

echo Done. Press any key to exit ...

pause
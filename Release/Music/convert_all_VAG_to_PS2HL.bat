@echo off

echo Unpatching all *.VAG music files in current folder ...

echo __________________________________________________

for %%I in (*.VAG) do mustool.exe unpatch %%I

echo __________________________________________________

echo Done. Press any key to exit ...

pause
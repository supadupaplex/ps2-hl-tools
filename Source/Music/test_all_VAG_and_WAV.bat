@echo off

echo Testing all *.VAG music files in current folder ...

echo __________________________________________________

for %%I in (*.VAG) do mustool.exe test %%I

for %%I in (*.WAV) do mustool.exe test %%I

echo __________________________________________________

echo Done. Press any key to exit ...

pause
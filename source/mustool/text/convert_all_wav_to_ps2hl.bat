@echo off

echo Unpatching all *.WAV audio files in current folder ...

echo __________________________________________________

for %%I in (*.WAV) do mustool.exe unpatch %%I

echo __________________________________________________

echo Done. Press any key to exit ...

pause
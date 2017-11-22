@echo off

echo Patching all *.WAV audio files in current folder ...

echo __________________________________________________

for %%I in (*.WAV) do mustool.exe patch %%I

echo __________________________________________________

echo Done. Press any key to exit ...

pause
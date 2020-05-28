@echo off

echo Testing all *.nod files in current folder ...

echo __________________________________________________

for %%I in (*.nod) do nodtool.exe test ^"%%I^"

echo __________________________________________________

echo Done. Press any key to exit ...

pause
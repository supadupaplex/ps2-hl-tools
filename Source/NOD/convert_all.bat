@echo off

echo Converting all *.nod files in current folder ...

echo __________________________________________________

for %%I in (*.nod) do nodtool.exe  %%I

echo __________________________________________________

echo Done. Press any key to exit ...

pause
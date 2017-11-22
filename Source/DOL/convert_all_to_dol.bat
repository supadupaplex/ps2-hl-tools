@echo off

echo __________________________________________________

echo Converting all *.mdl models in current folder ...

for %%I in (*.mdl) do mdltool.exe  %%I

echo __________________________________________________

echo Done. Press any key to exit ...

pause
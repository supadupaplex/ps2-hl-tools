@echo off

echo __________________________________________________

echo Converting all *.dol models in current folder ...

for %%I in (*.dol) do mdltool.exe  %%I

echo __________________________________________________

echo Done. Press any key to exit ...

pause
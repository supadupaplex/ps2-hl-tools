@echo off

echo Converting all *.psi images in current folder ...

echo __________________________________________________

for %%I in (*.psi) do psitool.exe  %%I

echo __________________________________________________

echo Done. Press any key to exit ...

pause
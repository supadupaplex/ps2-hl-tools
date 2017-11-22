@echo off

echo Extracting all *.pak files in current folder ...

echo __________________________________________________

for %%I in (*.pak) do paktool.exe extract %%I

echo __________________________________________________

echo Done. Press any key to exit ...

pause
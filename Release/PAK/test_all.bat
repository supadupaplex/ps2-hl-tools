@echo off

echo Showing information about all *.pak files in current folder ...

echo __________________________________________________

for %%I in (*.pak) do (echo ==========%%I========== & paktool.exe test %%I)

echo __________________________________________________

echo Done. Press any key to exit ...

pause
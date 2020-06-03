@echo off

echo Converting all PS2 Half-life Decals in current folder ...

echo __________________________________________________

for %%I in (*.) do phdtool.exe altbmp %%I

echo __________________________________________________

echo Done. Press any key to exit ...

pause

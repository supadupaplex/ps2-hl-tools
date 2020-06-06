@echo off

echo Reporting animation sequences from all models in current folder ...

echo __________________________________________________

for %%I in (*.mdl) do mdltool.exe seqrep %%I

for %%I in (*.dol) do mdltool.exe seqrep %%I

echo __________________________________________________

echo Done. Press any key to exit ...

pause
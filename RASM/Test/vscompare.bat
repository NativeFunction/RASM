@echo off
setlocal
set vspath=C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\Common7\IDE
start "Compare files" /B /MIN "%vspath%\devenv.exe" /diff %2 %1 First:'%2' Second:'%1'

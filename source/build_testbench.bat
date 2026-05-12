@echo off

REM if not exist build ( mkdir build )
cd build

echo:
echo Building testbench...

del *testbench* /Q > temp.txt

REM Link with the debug version of the API
echo:
copy ..\..\apad_api_lib64\*debug.* .

echo:
cl /I..\..\apad_api_lib64 /std:c++17 /w /nologo /Od /Zi ..\testbench.cpp /link *debug*.lib

del testbench.obj
del testbench.ilk
del temp.txt

echo:
echo Done

echo:


@echo off

REM if not exist build ( mkdir build )
cd build

echo:
echo Building testbench...

del *testbench* /Q > temp.txt

REM Link with the debug version of the API
echo:
copy ..\..\apad_api_lib64\bin\*debug.* .

echo:
cl /I..\..\apad_api_lib64\source /std:c++17 /w /nologo /Od /Zi /Fe: run_testbench.exe ..\testbench.cpp /link *debug*.lib

del *testbench*.obj
del *testbench*.ilk
del temp.txt

echo:
echo Done

echo:

REM Build rebuild_testbench.bat
echo @echo off > rebuild_testbench.bat
echo: >> rebuild_testbench.bat
echo cd .. >> rebuild_testbench.bat
echo call build_testbench.bat >> rebuild_testbench.bat
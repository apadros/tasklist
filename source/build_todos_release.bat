@echo off

if not exist build ( mkdir build )
cd build

del * /q

cl /nologo /w /I..\..\apad_api_lib64\source /Fe: todos /O2 /std:c++17 ..\helpers.cpp ..\todos.cpp ..\..\apad_api_lib64\bin\*debug.lib

if not exist release ( mkdir release )
del release\* /q
cd release
mkdir data
move ..\todos.exe .
copy ..\..\..\apad_api_lib64\bin\*debug.dll .

del ..\*.obj

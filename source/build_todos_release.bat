@echo off

if not exist build ( mkdir build )
cd build

del * /q

cl /nologo /w /I..\..\apad_api_lib64 /Fe: todos /O2 /std:c++17 ..\main.cpp ..\..\apad_api_lib64\*.lib

move todos.exe ..\..

cd ..\..

del *.dll
copy apad_api_lib64\*.dll .
del *debug.dll
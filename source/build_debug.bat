@echo off

if not exist build ( mkdir build )
cd build

del * /q

copy ..\..\apad_api_lib64\*debug.* .

cl /nologo /w /I..\..\apad_api_lib64 /Fe: todos /Od /Zi /DAPAD_DEBUG /std:c++17 ..\main.cpp *debug.lib
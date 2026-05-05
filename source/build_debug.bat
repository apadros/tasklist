@echo off

if not exist build ( mkdir build )
cd build

move rebuild_debug.bat ..
del * /q
move ..\rebuild_debug.bat .

copy ..\..\apad_api_lib64\*debug.* .

cl /nologo /w /I..\..\apad_api_lib64 /Fe: todos /Od /Zi /DAPAD_DEBUG /std:c++17 ..\main.cpp *debug.lib
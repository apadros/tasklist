@echo off

if not exist build ( mkdir build )
cd build

del * /q

copy ..\..\apad_api_lib64\bin\*debug.* .

cl /nologo /w /I..\..\apad_api_lib64\source /Fe: todos /Od /Zi /DAPAD_DEBUG_COMMANDS /DAPAD_DEBUG_DATA_PATH /std:c++17 ..\helpers.cpp ..\todos.cpp *debug.lib

del *.ilk
del *.obj

REM Build rebuild_debug.bat
echo @echo off > rebuild_todos_debug.bat
echo: >> rebuild_todos_debug.bat
echo cd .. >> rebuild_todos_debug.bat
echo call build_todos_debug.bat >> rebuild_todos_debug.bat
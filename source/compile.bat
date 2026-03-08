@echo off

if not exist build ( mkdir build )
pushd build

cl /I..\..\apad_api_lib64 /c /w /nologo /std:c++17 ..\main.cpp

popd
exit /b
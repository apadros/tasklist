@echo off

if not exist build ( mkdir build )
pushd build

cl /nologo /w /I..\..\apad_api_lib64 /std:c++17 /Fetodos /Od /Zi ..\main.cpp /link ..\..\apad_api_lib64\*.lib

popd
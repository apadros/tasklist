@echo off

if not exist build ( mkdir build )
pushd build

cl /I..\..\lib64 /c /w /nologo ..\main.cpp

popd
exit /b
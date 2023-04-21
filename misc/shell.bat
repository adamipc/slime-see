@echo off

CALL "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64 10.0.22621.0

set PATH=%PATH%;W:\handmade\misc\

pwsh.exe

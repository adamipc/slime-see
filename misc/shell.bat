@echo off

CALL "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64

set PATH=%PATH%;W:\handmade\misc\

pwsh.exe

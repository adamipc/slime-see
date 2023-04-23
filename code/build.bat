@echo off

set opts=-DENABLE_ASSERT=1
set cl_opts=-FAsc -FC -Zi -nologo -Fewin32_slime_see
set clang_opts=-o win32_slime_see.exe
set libs=user32.lib
set build_dir=..\..\..\build
IF NOT EXIST %build_dir% mkdir %build_dir%
pushd %build_dir%

set CODE_PATH=..\shaders\slime-see\code
cl %cl_opts% %opts% %CODE_PATH%\win32_slime_see.cpp %libs%
rem clang %clang_opts% %opts% %CODE_PATH%\win32_slime_see.cpp -l %libs%

popd



@echo off

rem enable for bitmap logo loading
rem set opts=-DENABLE_ASSERT=1 -DLOGOS=0
set opts=-DENABLE_ASSERT=1 

set cl_opts=-Od -Oi -GR- -EHa- -MT -Gm- -FAsc -W4 -WX -wd4702 -wd4127 -wd4100 -wd4189 -wd4201 -wd4505 -FC -Zi -nologo -Fewin32_slime_see -Fmwin32_slime_see.map
set link_opts=/link -opt:ref -subsystem:windows
set clang_opts=-o win32_slime_see.exe
set libs=user32.lib gdi32.lib advapi32.lib winmm.lib ole32.lib
set build_dir=..\..\..\build
IF NOT EXIST %build_dir% mkdir %build_dir%
pushd %build_dir%

set CODE_PATH=..\shaders\slime-see\code
cl %cl_opts% %opts% %CODE_PATH%\win32_slime_see.cpp %link_opts% %libs%
rem clang %clang_opts% %opts% %CODE_PATH%\win32_slime_see.cpp -l %libs%

popd



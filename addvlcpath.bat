rem enableextensions and enabledelayedexpansion are needed for string substitution to work
setlocal ENABLEEXTENSIONS
setlocal ENABLEDELAYEDEXPANSION

rem set vlcpath to the path of %ProgramFiles% (or %ProgramFiles(x86)% if it exists)
set vlcpath=%ProgramFiles%\VideoLAN\VLC
if exist "%ProgramFiles(x86)%" set vlcpath=%ProgramFiles(x86)%\VideoLAN\VLC

rem use string substitution to repace all occurances of "%vlcpath%;" with ""
set sdpath=!path:%vlcpath%;=!

rem append ";%vlcpath%;" to end of sdpath
rem leading ; is necessary in case path does not have a terminating ;
rem trailing ; is necessary for safe removal of vlcpath later
set sdpath=%sdpath%;%vlcpath%;

rem use string substitution to replace all occurances of ";;" with ";"
rem because I might now have a double ; before vlcpath
set sdpath=%sdpath:;;=;%

rem store the modified path environment variable in the registry
reg add "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v Path /t REG_EXPAND_SZ /d "%sdpath%" /f
rem a reboot will be necessary after modifying the path environment variable in the registry

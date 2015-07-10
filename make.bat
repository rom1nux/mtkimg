@echo off
rem ###########################################################################
rem                            MTKIMG PROJECT
rem ###########################################################################
rem file	make.bat
rem author	rom1nux
rem date 	2015-07-09
rem brief 	MINGW32 make wrapper for windows
rem ###########################################################################

rem Check for MINGW32 make binary
where mingw32-make 1>NUL 2>NUL
if %ERRORLEVEL%==0 goto makedo
echo.
echo ERROR : MINGW (mingw32-make) not found in your PATH !
echo You can download and install MINGW from "http://mingw-w64.sourceforge.net"
echo Do not forget to set MINGW bin directory in your PATH.
echo.
pause
exit

rem Running make
:makedo
mingw32-make %1%


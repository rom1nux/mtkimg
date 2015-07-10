@echo off
rem ###########################################################################
rem                            MTKIMG PROJECT
rem ###########################################################################
rem file	cygterm.bat
rem author	rom1nux
rem date 	2015-07-09
rem brief 	CYGWIN terminal launcher
rem ###########################################################################

rem Script parameters (Adjust CYGWIN path or LC_ALL if need)
set PATH=E:\cygwin64\bin;%PATH%
set LC_ALL=en_US.ISO-8859-1

rem Check for CYGWIN binaries
where cygstart 1>NUL 2>NUL
if %ERRORLEVEL%==0 goto cygwindo
echo.
echo ERROR : CYGWIN (cygstart) not found in your PATH !	
echo You can download and install CYGWIN from "http://cygwin.com/install.html"
echo Edit this script or set CYGWIN bin directory in your PATH.
echo.
pause
exit

rem Launch CYGWIN terminal
:cygwindo
rem Getting current directory
set CWD=%~dp0
rem Starting CYGWIN terminal in current directory
echo Starting CYGWIN in %CWD%...
start mintty.exe -i /Cygwin-Terminal.ico /bin/sh -lc 'export LC_ALL=%LC_ALL%; cd "`cygpath "%CWD%"`"; bash'


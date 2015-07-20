@echo off
rem ***************************************************************************
rem                            MTKIMG PROJECT
rem ***************************************************************************
rem brief		CYGWIN terminal launcher
rem file		cygterm.bat
rem author		rom1nux
rem date		2015-07-09
rem copyright	Copyright (C) 2015 rom1nux
rem ***************************************************************************
rem This program is free software: you can redistribute it and/or modify
rem it under the terms of the GNU General Public License as published by
rem the Free Software Foundation, either version 3 of the License, or
rem (at your option) any later version.
rem 
rem This program is distributed in the hope that it will be useful,
rem but WITHOUT ANY WARRANTY; without even the implied warranty of
rem MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
rem GNU General Public License for more details.
rem 
rem You should have received a copy of the GNU General Public License
rem along with this program.  If not, see <http://www.gnu.org/licenses/>.
rem ***************************************************************************

rem Script parameters (Adjust CYGWIN path or LC_ALL if need)
set PATH=E:\cygwin32\bin;%PATH%
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


@echo off
rem ***************************************************************************
rem                            MTKIMG PROJECT
rem ***************************************************************************
rem brief 		MINGW32 make wrapper for windows
rem file		make.bat
rem author		rom1nux
rem date 		2015-07-09
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


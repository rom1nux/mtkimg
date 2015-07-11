                 __  __ _______ _  _______ __  __  _____ 
                |  \/  |__   __| |/ /_   _|  \/  |/ ____|
                | \  / |  | |  |   /  | | | \  / | |  __ 
                | |\/| |  | |  |  <   | | | |\/| | | |_ |
                | |  | |  | |  |   \ _| |_| |  | | |__| |
                |_|  |_|  |_|  |_|\_\_____|_|  |_|\_____|
                                          
                       Copyright (c) 2015 rom1nux
					   
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.						   
					   
					   
A - HOW TO USE				   
					   
    Be sure "find", "gzip" and "cpio" are installed on your system and are in your PATH.
	
    Linux   : Use your package manager to download application
    Cygwin  : Use Cygwin setup.exe to install packages
    Windows : You can manually download package from gnuwin
              http://gnuwin32.sourceforge.net/packages/findutils.htm
              http://gnuwin32.sourceforge.net/packages/gzip.htm
              http://gnuwin32.sourceforge.net/packages/cpio.htm
			  		  
    On Linux or Cygwin hit "./mtkimg", on Windows hit "mtkimg.exe" to see full help.
   
    To unpack image hit :   ./mtkimg unpack boot.img
    To repack image hit :   ./mtkimg repack newboot.img

	
B - CHANGELOG

	2015-07-10 - rom1nux
	 * Add 'srcdist' and 'bindist' to Makefile
	 
    2015-07-09 - rom1nux
     * First beta
	 
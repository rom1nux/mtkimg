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

  
A - INTRODUCTION

  MTKIMG is a simple and portable utility to manage MTK Android ROM image file
  like boot.img, recovery.img or logo.bin. 
  
  Characteristics :

   * Write in C, open-source.
   * Portable : Linux, Cygwin, Windows (MAC is currently under port).
   * Only one executable for unpack/repack.   
   * Support unpack/repack boot.img and recovery.img.
   * Support unpack/repack logo.bin   
   * Keep the original image size at repack time.
   * Automatic ramdisk decompression.
   * Option to keep or remove MTK headers.
   * Check for valid "ARM Linux zImage" kernel.
   * Check for valid "GZIP" ramdisk.
   * Handle kernel and ramdisk with or without MTK header.
   * Check for overflow if kernel or ramdisk is too big at repack time.
   * Option to set compression rate for ramdisk.   
   * Easy to use, all parameters are defaulted.
   * Easy to build on every platform.
   * Got simple test-suite.
   * Documentation available.

  Releases :

   * mtkimg-0.43-linux64.zip       : Linux 64 bits binaries
   * mtkimg-0.43-cygwin64.zip      : Cygwin 64 bits binaries
   * mtkimg-0.43-osx64.zip         : Mac OSX 64 bits binaries
   * mtkimg-0.43-win64.zip         : Windows 64 bits binaries only
   * mtkimg-0.43-win64-bundle.zip  : Windows 64 bits binaries with externals utilities
   * mtkimg-win64-utilities.zip    : Windows 64 bits externals utilities only
   
   
B - HOW TO USE   

  Be sure "find", "gzip", "cpio" and "chmod" utilities are installed on your system 
  and are in your PATH environment variable :

   Linux   : Use your package manager to download application.
   Osx     : Use your package manager to download application.
   Cygwin  : Use Cygwin setup.exe to install packages.
   Windows : If you dont have the bundle version of MTKIMG, download 
             "mtkimg-win64-utilities.zip" extract files in the directory of "mtkimg.exe"

  Launch a terminal, cd into directory where "mtkimg" (mtkimg.exe) is, and execute MTKIMG 
  without arguments to see full help/usage. (On Windows bundle version, you can start 
  terminal by click on "winterm.bat" that add externals utilities automatically in your
  PATH environment variable for the current session.)

  On Linux, Cygwin or Osx hit "./mtkimg", on Windows hit "mtkimg.exe" to see full help.  

  Basically : 
  
   To unpack boot image execute :   ./mtkimg unpack boot.img
   To repack boot image execute :   ./mtkimg repack newboot.img

   To unpack logo image execute :   ./mtkimg unpack -t logo logo.bin
   To repack logo image execute :   ./mtkimg repack -t logo newlogo.bin
  
    
C - SOURCE CODE   

  Code source is multi-platform (Linux, Cygwin, Windows) and got several utilities
  to build, test, backup and distribute executable. It can be download from :
  https://github.com/rom1nux/mtkimg
  
  mtkimg
    |- build               : Build directory (output of compilation)
    |- doc                 : Documents directory
    |   `-html             : HTML source code documentation directory
    |      `- index.html   : Index of source code documentation
    |- release             : Release packages
    |- src                 : Sources code directory
    |   `- readme.txt      : Distrib readme file
    |- tools               : Utilities directory
    |   `- mtkimgtest.sh   : Automatic test suite to check building (Linux/Cygwin only)
    |- .gitignore          : GIT files/directory ignore
    |- cyterm.bat          : Cygwin terminal launcher
    |- licence.txt         : GPL3 license file
    |- make.bat            : Windows make wrapper (invoke "mink32-make.exe')
    |- Makefile            : Source code builder and management script
    |- readme.txt          : This help file
    `- winterm.bat         : Windows terminal launcher

  
D - HOW TO BUILD SOURCE CODE 

  First, be sure you got a "gcc" compiler and "make" utility are installed on your system 
  and be sure /bin directory is in your PATH environment variable.  
  
  Second, MTKIMG use "zlib" and "libpng" for logo unpack/repack to work.
  Be sure "zlib-dev" (zlib-devel) and "libpng-dev" (libpng-devel) are installed on your
  system.
  
  Note for Windows:
   You need MinGW32 or MinGW64 (See UTILTIES FOR BUILDING SOURCE CODE section). 
   MinGW contain a port of "make" utility named "ming32-make" invoked by "make.bat'. 
   You need to manually build "zlib" and "libpng" first before building MTKIMG.
   (see HOW TO BUILD ZLIB AND LIBPNG ON WINDOWS section) .
  
  Here is the few steps to follow to build MTKIMG on your platform :

   1 - Uncompress source code or pull source code from GitHub.
      
   2 - Open terminal in source code directory :
       Linux   : Open terminal
       Cygwin  : From Windows click on "cygterm.bat"
       Windows : Click on "winterm.bat"

   3 - Execute : make
      
   4 - Execute : make clean
      
   5 - "mtkimg" (mtkimg.exe) is ready into "build" directory.

   6 - You can copy and paste it into your bin directory or the working directory
       you are working in.

  Look at "src/readme.txt" for more information about using mtkimg.

  
E - HOW TO BUILD ZLIB AND LIBPNG ON WINDOWS

  Understand "zlib" need to be build before "libpng".
  Create the "externals" directory into MTKIMG source directory.
  Respect the name of directories, MTKIMG Makefile use this path for building.

  For zlib :
     
   1 - Download and unpack the latest "zlib" into "externals/zlib" directory.       
   2 - Click on "winterm.bat"
   3 - Execute : cd externals/zlib
   4 - Execute : make-ming32 -f win32/Makefile.gcc
   5 - "zlib.h" and "libz.a" are ready into "externals/zlib" directory.
   
  For libpng :
  
   1 - Download and unpack the latest "libpng" into "externals/libpng" directory.
   2 - Edit "externals/libpng/scripts/Makefile.gcc" with text editor
       At line ~19 : Change "CP = cp" to "CP = copy" 
       At line ~35 : Change the slash to backslash (scripts/... to scripts\...)
   3 - Click on "winterm.bat"
   4 - Execute : cd externals/libpng
   5 - Execute : make-ming32 -f scripts/Makefile.gcc   
   6 - "png.h" and "libpng.a" are ready into "externals/libpng" directory
  
  
F - MAKEFILE COMMANDS

  Makefile is used to manage the source code on all platform.
  (On Windows make.bat invoke and pass command to mingw32-make)
  
  This is Makefile supported command :
  
   make          : Build MTKIMG executable in ./build directory
   make clean    : Remove all object files (.o) from ./build directory
   make mrproper : Remove the build directory
   make rebuild  : Force rebuild all object files
   make doc      : Invoke Doxygen to build ./doc/html source documentation
   make release  : Create binary distribution zip into ./release directory
   make distro   : Create source distribution zip into ./release directory
   make backup   : Create a source zip backup  into ../bkp
   
   
G - UTILTIES FOR BUILDING SOURCE CODE

  * MINGW32 or MINGW64 (Windows GCC compiler port)
    http://sourceforge.net/projects/mingw-w64

  * ZLIB (Compression library)
    http://www.zlib.net

  * LIBPNG (Compression library)
    http://www.libpng.org
   
  * DOXYGEN (Multiplatform source code documentation generator)
    http://www.doxygen.org  

  * 7-ZIP (Windows compression/decompression utilities)
    http://http://www.7-zip.org


H - WHERE TO SPEAK ABOUT

  * GitHub official repository
    https://github.com/rom1nux/mtkimg

  * XDA Developers forum
    http://forum.xda-developers.com/android/development/tools-unpack-repack-boot-img-utility-t3154621

  * FrAndroid forum
    http://forum.frandroid.com/topic/223380-outils-unpackrepack-bootimg-mediatek
  
  
I - CHANGELOG

  * 2015-07-19 - V0.43 - rom1nux
    - Add Repack logo
    - Add image.cfg type checking before repack
    - Replace --version by --release
    - Add --version command and option to see detailed version
    - Change verbose/debug behavior
    - Change image layout presentation
    
  * 2015-07-19 - V0.42 - rom1nux
    - Fix problem at repack when kernel already got MTK header

  * 2015-07-18 - V0.41 - rom1nux
    - Fix permission issue after repack on Windows version.
    - Replace GNU Windows utilities by Cygwin utilities to preserve permission.
    - Minor corrections.
  
  * 2015-07-17 - V0.40 - rom1nux
    - Fix kernel/ramdisk pages count.
    - Fix overflow detection if kernel and/or ramdisk is too big.

  * 2015-07-16 - V0.38 - rom1nux
    - Unpack logo fully functional. (Repack in progress)
    - Add procedure to build zlib and libpng on Windows.
    - Makefile modification.

  * 2015-07-15 - V0.36 - rom1nux
    - Begin to start Apple OSX port support. (Thanks sambwel for your help)
  
  * 2015-07-13 - V0.34 - rom1nux
    - Start logo unpacking function. (not completed yet)
    - Documentation correction.

  * 2015-07-11 - V0.33 - rom1nux
    - Create GitHub repository.
    - Add source code under GPL3 license.

  * 2015-07-10 - V0.32 - rom1nux
    - Add "srcdist" and "bindist" to Makefile.
 
  * 2015-07-09 - V0.31 - rom1nux
    - First beta.
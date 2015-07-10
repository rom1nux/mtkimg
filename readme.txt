                 __  __ _______ _  _______ __  __  _____ 
                |  \/  |__   __| |/ /_   _|  \/  |/ ____|
                | \  / |  | |  |   /  | | | \  / | |  __ 
                | |\/| |  | |  |  <   | | | |\/| | | |_ |
                | |  | |  | |  |   \ _| |_| |  | | |__| |
                |_|  |_|  |_|  |_|\_\_____|_|  |_|\_____|
                                          
                       Copyright (c) 2015 rom1nux

A - INTRODUCTION

    MTKIMG is a simple and portable utility to manage MTK Android ROM image  

    mtkimg
      |- build               : Build directory (output of compilation)
      |- doc                 : Documents directory
      |   `-html             : HTML source code documentation directory
      |`- index.html         : Index of source code dcumentation
      |- src                 : Sources code directory
      |   `- readme.txt      : Distrib readme file
      |- tools               : Utilities directory
      |   `- mtkimgtest.sh   : Automatic test suite to check building (Linux/Cygwin only)
      |- cyterm.bat          : Cygwin terminal launcher 
      |- make.bat            : Windows make wrapper
      |- Makefile            : Source code builder and management script
      |- readme.txt          : This help file
      `- winterm.bat         : Windows terminal launcher

 
B - HOW TO BUILD

    1 - Uncompress source code
      
    2 - Open terminal in source code directory
        Linux   : Open terminal
        Cygwin  : From Windows click on "cygterm.bat"
        Windows : Click on "winterm.bat"

    3 - Execute : make
      
    4 - Execute : make clean
      
    5 - "mtkimg" (mtkimg.exe) is ready into "build" directory

    6 - You can copy and paste it into your bin directory or the working directory
	    you working on.

    Look at "src/readme.txt" for more information about using mtkimg.

   
   
   
   
    
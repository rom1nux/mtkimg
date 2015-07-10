                 __  __ _______ _  _______ __  __  _____ 
                |  \/  |__   __| |/ /_   _|  \/  |/ ____|
                | \  / |  | |  |   /  | | | \  / | |  __ 
                | |\/| |  | |  |  <   | | | |\/| | | |_ |
                | |  | |  | |  |   \ _| |_| |  | | |__| |
                |_|  |_|  |_|  |_|\_\_____|_|  |_|\_____|
                                          
                       Copyright (c) 2015 rom1nux
					   
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
	 
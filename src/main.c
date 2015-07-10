/** ***************************************************************************
  *                          MTKIMG PROJECT
  * ***************************************************************************
  * \file		main.h
  * \author		rom1nux
  * \version	1.0
  * \date		2015-07-09
  * \brief		Main file
  * ***************************************************************************
  */
#include "main.h"

/**
 * \brief		Main entry point
 * \param		argc 		Command line arguments count 
 * \param		argv 		Command line arguments
 * \return 		Operation status
 * \retval 		true		Success
 * \retval 		false		Fail
 */	
int main(int argc, char** argv)
{		
	char* ptr;
	int i;
	args_t args;
	 	
	// Init data
	memset(&app_data,0x0,sizeof(app_data_t));
	// Extract exe name
	ptr=strrchr(argv[0],FILESEP);
	strcpy(app_data.exename,(ptr ? ++ptr : argv[0]));
	// Get working directory
	dir_get_current(app_data.working_dir);
	// Init args
	args_init(&args);
	
	// Parsing command line
	for(i=1;i<argc;i++){
		if (argv[i][0]=='-'){
			if ((!strcmp(argv[i],"--help")) || (!strcmp(argv[i],"-h"))){ strcpy(app_data.cmd,CMD_HELP); break; }
			else if ((!strcmp(argv[i],"--version")) || (!strcmp(argv[i],"-V"))){ printf("%s-%s-%s%d\n",APP_NAME,APP_VERSION,APP_PLATFORM,APP_ARCH); exit(0); }
			else if ((!strcmp(argv[i],"--verbose")) || (!strcmp(argv[i],"-v"))) app_data.verbose=true;
			else if ((!strcmp(argv[i],"--debug")) || (!strcmp(argv[i],"-d"))) app_data.debug=true;
			else args_add(&args,argv[i]); 
		}else{
			if (!app_data.cmd[0]) strcpy(app_data.cmd,argv[i]);	
			else args_add(&args,argv[i]);			
		}
	}
	// Default command 'help'
	if (app_data.cmd[0]==0) strcpy(app_data.cmd,CMD_HELP);

	// Banner
	printf("%s V%s(%s%d) by %s\n",APP_TITLE,APP_VERSION,APP_PLATFORM,APP_ARCH,APP_AUTHOR);
	
	// Dispatch command
	debug("Execute command '%s'...",app_data.cmd);
	if (!strcmp(app_data.cmd,CMD_HELP)){
		show_usage();
	}else if (!strcmp(app_data.cmd,CMD_INFO)){
		info(&args);
	}else if (!strcmp(app_data.cmd,CMD_UNPACK)){
		unpack(&args);
	}else if (!strcmp(app_data.cmd,CMD_REPACK)){
		repack(&args);
	}else{
		die("Unknown command '%s' !",app_data.cmd);
	}
	
	// Free args
	args_free(&args);	
	return 0;
}

/**
 * \brief		Display application usage
 */	
void show_usage()
{
	putchar('\n');
	printf("Usage :\n\n");
	printf(" %s <command> [imgfile] [options]\n",app_data.exename); 
	printf("\nCommands :\n\n");
	printf(" help    : Show this help\n");
	printf(" info    : Show image information\n");
	printf(" unpack  : Unpack image file\n");
	printf(" repack  : Repack image file\n");
	printf("\nOptions :\n\n"); 
	printf(" --help            -h        : Show this help\n");
	printf(" --version         -V        : Show numerical version\n");
	printf(" --verbose         -v        : Show details\n");
	printf(" --debug           -d        : Show debug messages\n");
	printf(" --overwrite       -o        : Overwrite existing file and directory\n");
	printf(" --kernel          -k <file> : Set kernel filename\n");
	printf(" --ramdisk         -r <dir>  : Set ramdisk directory (or filename, see -n)\n");
	printf(" --config          -c <file> : Set configuration filename\n");
	printf(" --no-compress     -n        : Do not compress ramdisk (use --ramdisk as file)\n");
	printf(" --no-decompress   -n        : Do not decompress ramdisk (use --ramdisk as file)\n");
	printf(" --compress-rate   -z <rate> : Compression level (1-9)\n");
	printf(" --keep-mtk-header -m        : Keep MTK header \n");
	printf("\nNotes :\n\n"); 
	printf(" * If you use '--no-decompress', unpacked ramdisk is not decompressed\n");
	printf("   and '--ramdisk' must be a file instead of directory.\n\n");
	printf(" * If you use '--no-compress', '--ramdisk' must be a file instead of directory\n");
	printf("   and it will be packed directly to output image.\n\n");
	printf(" * Using '--keep-mtk-header' implies using '--no-decompress' option when unpack.\n\n");
	printf(" * Unpack command automatically detect if kernel or ramdisk got MTK header.\n\n");
	printf("Examples :\n\n"); 
	printf(" %s info myboot.img\n",app_data.exename);
	printf(" %s unpack\n",app_data.exename);
	printf(" %s unpack myboot.img -v\n",app_data.exename);
	printf(" %s unpack myboot.img -k mykernel.img -r myramdisk\n",app_data.exename);
	printf(" %s unpack myboot.img -n -k mykernel.img -r myramdisk.cpio.gz\n",app_data.exename);
	printf(" %s unpack myboot.img -n -m -k mykernel.img.mtk -r myramdisk.cpio.gz.mtk\n",app_data.exename);
	printf(" %s repack\n",app_data.exename);
	printf(" %s repack newboot.img -v\n",app_data.exename);
	printf(" %s repack newboot.img -k mykernel.img -r myramdisk\n",app_data.exename);
	printf(" %s repack newboot.img -n -k mykernel.img -r myramdisk.cpio.gz\n",app_data.exename);
	printf(" %s repack newboot.img -n -k mykernel.img.mtk -r myramdisk.cpio.gz.mtk\n",app_data.exename);
	exit(1);
}



/** ***************************************************************************
  *                          MTKIMG PROJECT
  * ***************************************************************************
  * \file		info.c
  * \author		rom1nux
  * \version	1.0
  * \date		2015-07-09
  * \brief		Info command
  * ***************************************************************************  
  */    
#include "info.h"

/**
 * \brief		Info command
 * \param		args 		Command line arguments
 */	
void info(args_t* args)
{
	info_data_t 	data;
	img_header_t	img_header;
	FILE*			fs=NULL;	
	uint32_t		kernel_offset,  kernel_pages;
	uint32_t		ramdisk_offset,  ramdisk_pages;	
	uint32_t 		img_size;

	// Init data
	memset(&data,0x0,sizeof(info_data_t));

	// Parse data	
	info_parse_args(&data, args);
	
	// Defaulting
	if (!data.input[0])	strcpy(data.input,DEFAULT_IMG_FILENAME);

	// Debug parsing/defaulting
	debug("%-20s : %s","verbose",bool2yn(app_data.verbose));
	debug("%-20s : %s","debug",bool2yn(app_data.debug));		
	debug("%-20s : %s","input",data.input);	
	
	// Check input file
	if (!file_exists(data.input)) die("Input file '%s' not found !",data.input);
	
	// Get input file size
	img_size=file_size(data.input);	
	
	// Openning input filename
	fs=fopen(data.input,"rb");
	if (fs==NULL) die("Could not open input file '%s' !",data.input);
	
	// Reading image header
	if (!img_header_read(&img_header,fs)) die("Could not read image header !");	
	img_header_show(&img_header);
	if (!img_header_check_signature(&img_header)) die("Android image signature is not valid !");

	// Compute offsets
	kernel_offset=img_header.page_size;
	kernel_pages=(long)ceil(img_header.kernel_size/img_header.page_size)+1;	
	ramdisk_offset=(kernel_pages+1)*img_header.page_size;
	ramdisk_pages=(long)ceil(img_header.ramdisk_size/img_header.page_size);
	
	// Show layout
	printf("\nImage layout :\n\n");
	printf(" %-30s : %d bytes\n","Image size",img_size);
	printf(" %-30s : 0x%08X\n","Kernel offset",kernel_offset);
	printf(" %-30s : %-20d (%d bytes)\n","Kernel pages",kernel_pages,kernel_pages*img_header.page_size);
	printf(" %-30s : 0x%08X\n","Ramdisk offset",ramdisk_offset);
	printf(" %-30s : %-20d (%d bytes)\n","Ramdisk pages",ramdisk_pages,ramdisk_pages*img_header.page_size);

	// Closing input filename
	fclose(fs);
	
}

/**
 * \brief		Info command argument parser
 * \param		data 		Data to fill
 * \param		args 		Arguments to parse
 */	
void info_parse_args(info_data_t* data, args_t* args)
{
	unsigned int i, nargs=1;
	// Parse all arguments
	debug("Parsing arguments...");
	for(i=0;i<args->argc;i++){
		if (args->argv[i][0]=='-'){
			// Options
			die("Unknown option '%s' (%d) for command '%s' !",args->argv[i],i,CMD_INFO);
		}else{
			// Arguments
			if (nargs==1) strcpy(data->input,args->argv[i]);				
			else die("Unknown argument '%s' (%d) !",args->argv[i],i);
			nargs++;
		}
	}
}

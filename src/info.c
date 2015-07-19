/** ***************************************************************************
  *                          MTKIMG PROJECT
  * ***************************************************************************
  * \brief		Info command
  * \file		info.c
  * \author		rom1nux
  * \date		2015-07-09
  * \copyright	Copyright (C) 2015 rom1nux   
  * ***************************************************************************
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  * 
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  * 
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.  
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
	uint32_t 		img_size, total_pages, total_size;

	// Init data
	memset(&data,0x0,sizeof(info_data_t));

	// Parse data	
	info_parse_args(&data, args);
	
	// Defaulting
	if (!data.input[0])	strcpy(data.input,DEFAULT_BOOT_IMG_FILENAME);

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
	kernel_pages=(long)ceil(img_header.kernel_size/(double)img_header.page_size);
	ramdisk_offset=(kernel_pages+1)*img_header.page_size;
	ramdisk_pages=(long)ceil(img_header.ramdisk_size/(double)img_header.page_size);
	total_pages=(kernel_pages+ramdisk_pages+1);
	total_size=total_pages*img_header.page_size;
	
	// Show layout
	output("\nImage layout :\n");	
	output(" %-10s %10s %10s %15s","Bloc","Offset","Pages", "Size (bytes)");		
	output(" ------------------------------------------------");
	output(" %-10s 0x%08X %10d %15d","Header",0,1,img_header.page_size);
	output(" %-10s 0x%08X %10d %15d","Kernel",kernel_offset,kernel_pages,kernel_pages*img_header.page_size);
	output(" %-10s 0x%08X %10d %15d","Ramdisk",ramdisk_offset,ramdisk_pages,ramdisk_pages*img_header.page_size);
	output(" %-10s 0x%08X %10d %15d","Free",ramdisk_offset+(ramdisk_pages*img_header.page_size),(img_size/img_header.page_size)-total_pages,img_size-total_size);
	output(" ------------------------------------------------");
	output(" %-10s %10s %10d %15d","Total","",img_size/img_header.page_size,img_size);
	output(" ------------------------------------------------");
	output("\n");	

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
			die("Unknown option '%s' for command '%s' !",args->argv[i],CMD_INFO);
		}else{
			// Arguments
			if (nargs==1) strcpy(data->input,args->argv[i]);				
			else die("Unknown argument '%s' !",args->argv[i]);
			nargs++;
		}
	}
}

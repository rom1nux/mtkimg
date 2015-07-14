/** ***************************************************************************
  *                          MTKIMG PROJECT
  * ***************************************************************************
  * \brief		Repack command
  * \file		repack.c
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
#include "repack.h"

/**
 * \brief		Repack command
 * \param		args 		Command line arguments
 */	
void repack(args_t* args)
{
	repack_data_t	data;	
	img_cfg_t		img_cfg;		
	img_header_t	img_header;
	mtk_header_t	mtk_header;	
	FILE*			fd=NULL;	
	uint32_t		kernel_offset, kernel_size, kernel_pages;
	uint32_t		ramdisk_offset, ramdisk_size, ramdisk_pages;
	uint32_t		min_img_size;
	bool			kernel_is_mtk_file,ramdisk_is_mtk_file;
	char*			ramdisk_file;
	char 			syscmd[CMD_MAX_SIZE];
	int				res;	
		
	
	// Init
	memset(&data,0x0,sizeof(repack_data_t));
	memset(&img_cfg,0x0,sizeof(img_cfg_t));
	memset(&img_header,0x0,sizeof(img_header_t));
	
	// Parse data
	repack_parse_args(&data, args);
	
	// Defaulting
	if (!data.compress_rate) data.compress_rate=DEFAULT_COMPRESS_RATE;
	if (!data.output[0]) strcpy(data.output,DEFAULT_BOOT_IMG_FILENAME);			
	if (!data.config[0]) strcpy(data.config,DEFAULT_CONFIG_FILENAME);	
	if (!data.kernel[0]) strcpy(data.kernel,DEFAULT_KERNEL_FILENAME);		
	if (!data.ramdisk[0]) strcpy(data.ramdisk,( data.no_compress ? DEFAULT_RAMDISK_FILENAME : DEFAULT_RAMDISK_DIR ));
	
	// Debug parsing/defaulting
	debug("%-20s : %s","verbose",bool2yn(app_data.verbose));
	debug("%-20s : %s","debug",bool2yn(app_data.debug));	
	debug("%-20s : %s","overwrite",bool2yn(data.overwrite));
	debug("%-20s : %s","output",data.output);
	debug("%-20s : %s","kernel",data.kernel);
	debug("%-20s : %s","ramdisk",data.ramdisk);
	debug("%-20s : %s","config",data.config);	
	debug("%-20s : %s","no-compress",bool2yn(data.no_compress));	
	debug("%-20s : %d","compress-rate",data.compress_rate);	
	
	// Check input file
	if (!file_exists(data.kernel)) die("Input file '%s' not found !",data.kernel);
	if (data.no_compress){
		if (!dir_exists(data.ramdisk)) die("Input directory '%s' not found !",data.ramdisk);
	}else{
		if (!file_exists(data.ramdisk)) die("Input file '%s' not found !",data.ramdisk);
	}		
	if (!file_exists(data.config)) die("Input file '%s' not found !",data.config);

	// Check for existing output files
	if (!data.overwrite){			
		if (file_exists(data.output)) die("File '%s' already exist !",data.output);
	}
	
	// Clean previous output files
	if (!file_remove(TMP_RAMDISK_FILENAME)) die("Could not remove file '%s' !",TMP_RAMDISK_FILENAME);
	if (!file_remove(data.output)) die("Could not remove file '%s' !",data.output);
	
	// Banner
	output("Repacking image to '%s'...",data.output);
	
	// Compress ramdisk
	if (!data.no_compress){
		output("Packing ramdisk content from '%s'...",data.ramdisk);
		// Entering directory
		if (!dir_change(data.ramdisk)) fail("Could not change to directory '%s' !",data.ramdisk);
		// Create compress command line
		sprintf(syscmd,"%s . | %s -o -H newc --quiet | %s -%d -n > ..%c%s",FIND_BIN,CPIO_BIN,GZIP_BIN,data.compress_rate,FILESEP,TMP_RAMDISK_FILENAME); 
		// Decompression
		verbose("Packing ramdisk content...");
		verbose("%s",syscmd);
		res=system(syscmd);
		// Returning to previous directory
		if (!dir_change(app_data.working_dir)) fail("Could not change to directory '%s' !",data.ramdisk);
		if (res) fail("Could not pack ramdisk content from directory '%s' !",data.ramdisk);		
	}
	
	// Ramdisk filename helper
	ramdisk_file=(data.no_compress?data.ramdisk:TMP_RAMDISK_FILENAME);
	
	// Detect kernel MTK header or ARM Linux zImage
	kernel_is_mtk_file=is_valid_mtk_file(data.kernel);	
	if (kernel_is_mtk_file){
		verbose("File '%s' is MTK file !",data.kernel);		
	}else{
		if (!is_valid_zimage_file(data.kernel)) fail("Kernel file '%s' is not a valid ARM Linux zImage file !",data.kernel);
		verbose("File '%s' is ARM Linux zImage file !",data.kernel);		
	}
	
	// Detect ramdisk MTK header
	ramdisk_is_mtk_file=is_valid_mtk_file(ramdisk_file);	
	if (ramdisk_is_mtk_file){
		verbose("File '%s' is MTK file !",ramdisk_file);	
	}else{
		// Check for valid gzip file
		if (!is_valid_gzip_file(ramdisk_file)) fail("Ramdisk file '%s' is not a valid gzip file !",ramdisk_file);
		verbose("File '%s' is GZIP file !",ramdisk_file);	
	}
	
	// Loading image configuration
	if (!img_cfg_read(&img_cfg, data.config)) fail("Could not load image configuration file '%s' !",data.config);	
	
	// Update image header from configuration	
	kernel_size=file_size(data.kernel);
	img_cfg.header.kernel_size=kernel_size + ( kernel_is_mtk_file ? 0 : sizeof(mtk_header_t) );
	kernel_offset=img_cfg.header.page_size;
	kernel_pages=(long)ceil(img_cfg.header.kernel_size/img_cfg.header.page_size)+1;
	ramdisk_size=file_size(ramdisk_file);
	img_cfg.header.ramdisk_size=ramdisk_size + ( ramdisk_is_mtk_file ? 0 : sizeof(mtk_header_t) );
	ramdisk_offset=(kernel_pages+1)*img_cfg.header.page_size;
	ramdisk_pages=(long)ceil(img_cfg.header.ramdisk_size/img_cfg.header.page_size);	
	memcpy(&img_header,&img_cfg.header,sizeof(img_header_t));
	min_img_size=(kernel_pages+ramdisk_pages+1)*img_header.page_size;
	
	// Show layout
	if (app_data.debug){
		img_header_show(&img_header);
		printf("\nImage layout :\n\n");
		printf(" %-30s : %d bytes\n","Image size",img_cfg.size);
		printf(" %-30s : 0x%08X\n","Kernel offset",kernel_offset);
		printf(" %-30s : %-20d (%d bytes)\n","Kernel pages",kernel_pages,kernel_pages*img_header.page_size);
		printf(" %-30s : 0x%08X\n","Ramdisk offset",ramdisk_offset);
		printf(" %-30s : %-20d (%d bytes)\n","Ramdisk pages",ramdisk_pages,ramdisk_pages*img_header.page_size);
		putchar('\n');
	}
	
	// Check valid image size
	if (min_img_size>img_cfg.size) die("Kernel and ramdisk are too big to fit into image !");
	
	// Openning output file
	fd=fopen(data.output,"wb");
	if (fd==NULL) fail("Could not open output file '%s' !",data.output);
		
	// Write image header
	if (!img_header_write(&img_header,fd)) fail("Could not write image header !"); 
		
	// Pad
	if (!pad_file(fd,img_header.page_size-sizeof(img_header_t),0x0)) fail("Could not pad file !");

	// KERNEL
	
	// Write kernel MTK header
	if (!kernel_is_mtk_file){
		memset(&mtk_header,0x0,sizeof(mtk_header_t));
		mtk_header.magic[0]=0x88; mtk_header.magic[1]=0x16;	mtk_header.magic[2]=0x88; mtk_header.magic[3]=0x58;	
		mtk_header.size=kernel_size;
		memcpy(mtk_header.type,"KERNEL",6);
		memset(&mtk_header.unused2,0xFF,472);
		if (!mtk_header_write(&mtk_header, fd)) fail("Could not create kernel MTK header !");
	}
	
	// Write kernel	
	output("Repacking kernel '%s' (%d bytes)...",data.kernel,kernel_size);
	if (!repack_file(fd,data.kernel)) fail("Could not repack kernel '%s' !",data.kernel);

	// Pad
	if (!pad_file(fd,(kernel_pages*img_header.page_size)-img_header.kernel_size,0x0)) fail("Could not pad file !");	

	// RAMDISK
	
	// Write ramdisk MTK header
	if (!kernel_is_mtk_file){
		memset(&mtk_header,0x0,sizeof(mtk_header_t));
		mtk_header.magic[0]=0x88; mtk_header.magic[1]=0x16;	mtk_header.magic[2]=0x88; mtk_header.magic[3]=0x58;	
		mtk_header.size=ramdisk_size;
		memcpy(mtk_header.type,img_cfg.type,32);
		memset(&mtk_header.unused2,0xFF,472);
		if (!mtk_header_write(&mtk_header, fd)) fail("Could not create ramdisk MTK header !");
	}
	
	// Write ramdisk
	output("Repacking ramdisk '%s' (%d bytes)...",ramdisk_file,ramdisk_size);
	if (!repack_file(fd,ramdisk_file)) fail("Could not repack ramdisk '%s' !",ramdisk_file);
	
	// Pad
	if (!pad_file(fd,img_cfg.size-(((kernel_pages+1)*img_header.page_size)+img_header.ramdisk_size),0x0)) fail("Could not pad file !");	
		
	// Closing output file
	fclose(fd);

	if (!file_remove(TMP_RAMDISK_FILENAME)) fail("Could not remove file '%s' !",TMP_RAMDISK_FILENAME);
	
	output("Image successfully repacked !");	
	return;
	
failed:
	// FAILED
	// Closing input file
	if (fd) fclose(fd);
	// Cleaning outputs	
	if (!file_remove(data.output)) error("Could not remove file '%s' !",data.output);
	if (!file_remove(TMP_RAMDISK_FILENAME)) error("Could not remove file '%s' !",TMP_RAMDISK_FILENAME);	
	exit(1);
}

/**
 * \brief		Repack command argument parser
 * \param		data 		Data to fill
 * \param		args 		Arguments to parse
 */	
void repack_parse_args(repack_data_t* data, args_t* args)
{
	unsigned int i, nargs=1;
	// Parse all arguments
	debug("Parsing arguments...");
	for(i=0;i<args->argc;i++){
		if (args->argv[i][0]=='-'){
			// Options
			if ((!strcmp(args->argv[i],"--overwrite")) || (!strcmp(args->argv[i],"-o"))){
				data->overwrite=true;
			}else if ((!strcmp(args->argv[i],"--kernel")) || (!strcmp(args->argv[i],"-k"))){
				if ((i+1)>=args->argc) die("Argument needed after option '%s' !",args->argv[i]);
				strcpy(data->kernel,args->argv[++i]);
			}else if ((!strcmp(args->argv[i],"--ramdisk")) || (!strcmp(args->argv[i],"-r"))){
				if ((i+1)>=args->argc) die("Argument needed after option '%s' !",args->argv[i]);
				strcpy(data->ramdisk,args->argv[++i]);							
			}else if ((!strcmp(args->argv[i],"--config")) || (!strcmp(args->argv[i],"-c"))){
				if ((i+1)>=args->argc) die("Argument needed after option '%s' !",args->argv[i]);
				strcpy(data->config,args->argv[++i]);
			}else if ((!strcmp(args->argv[i],"--no-compress")) || (!strcmp(args->argv[i],"-n"))){
				data->no_compress=true;		
			}else if ((!strcmp(args->argv[i],"--compress-rate")) || (!strcmp(args->argv[i],"-z"))){
				if ((i+1)>=args->argc) die("Argument needed after option '%s' !",args->argv[i]);
				data->compress_rate=strtol(args->argv[++i],NULL,10);				
			}else{
				die("Unknown option '%s' for command '%s' !",args->argv[i],CMD_UNPACK);
			}
		}else{
			// Arguments
			if (nargs==1) strcpy(data->output,args->argv[i]);		
			else die("Unknown argument '%s' !",args->argv[i]);
			nargs++;
		}
	}
}
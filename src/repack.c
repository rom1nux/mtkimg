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
void repack(args_t* args){
	repack_data_t	data;

	// Init	
	memset(&data,0x0,sizeof(repack_data_t));
	
	// Parse data
	repack_parse_args(&data, args);
	
	// Defaulting
	if (data.type==CMD_TYPE_UNKNOWN) data.type=CMD_TYPE_BOOT;
	
	// Dispatch command
	if (data.type==CMD_TYPE_BOOT){
		// Repack boot.img		
		repack_boot(&data);
	}else{	
		// Repack logo.bin
		repack_logo(&data);		
	}
}
 
 /**
 * \brief		Repack 'boot.img' command
 * \param		data 		Repack command data
 */	 
void repack_boot(repack_data_t* data)
{	
	img_cfg_t		img_cfg;		
	img_header_t	img_header;
	mtk_header_t	mtk_header;	
	FILE*			fd=NULL;	
	uint32_t		kernel_offset, kernel_size, kernel_pages;
	uint32_t		ramdisk_offset, ramdisk_size, ramdisk_pages;
	uint32_t		total_pages,total_size;
	bool			kernel_is_mtk_file,ramdisk_is_mtk_file;
	char*			ramdisk_file;
	char 			syscmd[CMD_MAX_SIZE];
	int				res;	
	
	// Init	
	memset(&img_cfg,0x0,sizeof(img_cfg_t));
	memset(&img_header,0x0,sizeof(img_header_t));
	
	// Defaulting
	if (!data->compress_rate) data->compress_rate=DEFAULT_COMPRESS_RATE;
	if (!data->output[0]) strcpy(data->output,DEFAULT_BOOT_IMG_FILENAME);			
	if (!data->config[0]) strcpy(data->config,DEFAULT_CONFIG_FILENAME);	
	if (!data->kernel[0]) strcpy(data->kernel,DEFAULT_KERNEL_FILENAME);		
	if (!data->ramdisk[0]) strcpy(data->ramdisk,( data->no_compress ? DEFAULT_RAMDISK_FILENAME : DEFAULT_RAMDISK_DIR ));
	
	// Debug parsing/defaulting
	debug("%-20s : %s","verbose",bool2yn(app_data.verbose));
	debug("%-20s : %s","debug",bool2yn(app_data.debug));	
	debug("%-20s : %s","overwrite",bool2yn(data->overwrite));
	debug("%-20s : %s","output",data->output);
	debug("%-20s : %s","kernel",data->kernel);
	debug("%-20s : %s","ramdisk",data->ramdisk);
	debug("%-20s : %s","config",data->config);	
	debug("%-20s : %s","no-compress",bool2yn(data->no_compress));	
	debug("%-20s : %d","compress-rate",data->compress_rate);	
	
	// Check input file
	if (!file_exists(data->kernel)) die("Input file '%s' not found !",data->kernel);
	if (data->no_compress){
		if (!dir_exists(data->ramdisk)) die("Input directory '%s' not found !",data->ramdisk);
	}else{
		if (!file_exists(data->ramdisk)) die("Input file '%s' not found !",data->ramdisk);
	}		
	if (!file_exists(data->config)) die("Input file '%s' not found !",data->config);

	// Check for existing output files
	if (!data->overwrite){			
		if (file_exists(data->output)) die("File '%s' already exist !",data->output);
	}
	
	// Clean previous output files
	if (!file_remove(TMP_RAMDISK_FILENAME)) die("Could not remove file '%s' !",TMP_RAMDISK_FILENAME);
	if (!file_remove(data->output)) die("Could not remove file '%s' !",data->output);
	
	// Banner
	output("Repacking image to '%s'...",data->output);
	
	// Loading image configuration
	if (!img_cfg_read(&img_cfg, data->config)) fail("Could not load image configuration file '%s' !",data->config);	
	
	// Check for config file type for 'LOGO'
	if 	(strcmp(img_cfg.type,"ROOTFS") && strcmp(img_cfg.type,"RECOVERY")) die("Configuration file '%s' is not a valid 'ROOTFS' or 'RECOVERY' image descriptor !",data->config);	
	
	// Compress ramdisk
	if (!data->no_compress){
		output("Packing ramdisk content from '%s'...",data->ramdisk);
#if defined(APP_WIN)		
		// Ensure permission are OK on Windows platform
		sprintf(syscmd,"%s 775 %s",CHMOD_BIN, data->ramdisk);
		verbose("Changing ramdisk directory permission...");
		verbose("%s",syscmd);
		if (system(syscmd)) fail("Could not change ramdisk directory '%s' permission !",data->ramdisk);
#endif		
		// Entering directory
		if (!dir_change(data->ramdisk)) fail("Could not change to directory '%s' !",data->ramdisk);
		// Create compress command line
		sprintf(syscmd,"%s . | %s -o -H newc --quiet | %s -%d -n > ..%c%s",FIND_BIN,CPIO_BIN,GZIP_BIN,data->compress_rate,FILESEP,TMP_RAMDISK_FILENAME); 
		// Decompression
		verbose("Packing ramdisk content (compression rate : %d)...",data->compress_rate);
		verbose("%s",syscmd);
		res=system(syscmd);
		// Returning to previous directory
		if (!dir_change(app_data.working_dir)) fail("Could not change to directory '%s' !",data->ramdisk);
		if (res) fail("Could not pack ramdisk content from directory '%s' !",data->ramdisk);		
	}
	
	// Ramdisk filename helper
	ramdisk_file=(data->no_compress?data->ramdisk:TMP_RAMDISK_FILENAME);
	
	// Detect kernel MTK header or ARM Linux zImage
	kernel_is_mtk_file=is_valid_mtk_file(data->kernel);	
	if (kernel_is_mtk_file){
		verbose("File '%s' is MTK file !",data->kernel);		
	}else{
		if (!is_valid_zimage_file(data->kernel)) fail("Kernel file '%s' is not a valid ARM Linux zImage file !",data->kernel);
		verbose("File '%s' is ARM Linux zImage file !",data->kernel);		
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
		
	// Update image header from configuration	
	kernel_size=file_size(data->kernel);
	img_cfg.header.kernel_size=kernel_size + ( kernel_is_mtk_file ? 0 : sizeof(mtk_header_t) );
	kernel_offset=img_cfg.header.page_size;
	kernel_pages=(long)ceil(img_cfg.header.kernel_size/(double)img_cfg.header.page_size);
	ramdisk_size=file_size(ramdisk_file);
	img_cfg.header.ramdisk_size=ramdisk_size + ( ramdisk_is_mtk_file ? 0 : sizeof(mtk_header_t) );
	ramdisk_offset=(kernel_pages+1)*img_cfg.header.page_size;
	ramdisk_pages=(long)ceil(img_cfg.header.ramdisk_size/(double)img_cfg.header.page_size);
	memcpy(&img_header,&img_cfg.header,sizeof(img_header_t));
	total_pages=kernel_pages+ramdisk_pages+1;
	total_size=total_pages*img_header.page_size;
	
	// Show layout
	if (app_data.verbose){
		img_header_show(&img_header);
		verbose("\nImage layout :\n");		
		verbose(" %-10s %10s %10s %15s","Bloc","Offset","Pages", "Size (bytes)");		
		verbose(" ------------------------------------------------");
		verbose(" %-10s 0x%08X %10d %15d","Header",0,1,img_header.page_size);
		verbose(" %-10s 0x%08X %10d %15d","Kernel",kernel_offset,kernel_pages,kernel_pages*img_header.page_size);
		verbose(" %-10s 0x%08X %10d %15d","Ramdisk",ramdisk_offset,ramdisk_pages,ramdisk_pages*img_header.page_size);
		verbose(" %-10s 0x%08X %10d %15d","Free",ramdisk_offset+(ramdisk_pages*img_header.page_size),(img_cfg.size/img_header.page_size)-total_pages,img_cfg.size-total_size);
		verbose(" ------------------------------------------------");
		verbose(" %-10s %10s %10d %15d","Total","",img_cfg.size/img_header.page_size,img_cfg.size);
		verbose(" ------------------------------------------------");
		verbose("");
	}
	
	// Check valid image size
	if (total_size>img_cfg.size) die("Kernel and/or ramdisk are too big to fit into image (%d bytes exceed) !",total_size-img_cfg.size);
	
	// Openning output file
	fd=fopen(data->output,"wb");
	if (fd==NULL) fail("Could not open output file '%s' !",data->output);
		
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
		if (app_data.verbose) mtk_header_show(&mtk_header);
	}
	
	// Write kernel	
	output("Repacking kernel '%s' (%d bytes)...",data->kernel,kernel_size);
	if (!repack_file(fd,data->kernel)) fail("Could not repack kernel '%s' !",data->kernel);

	// Pad
	if (!pad_file(fd,(kernel_pages*img_header.page_size)-img_header.kernel_size,0x0)) fail("Could not pad file !");	

	// RAMDISK
	
	// Write ramdisk MTK header
	if (!ramdisk_is_mtk_file){
		memset(&mtk_header,0x0,sizeof(mtk_header_t));
		mtk_header.magic[0]=0x88; mtk_header.magic[1]=0x16;	mtk_header.magic[2]=0x88; mtk_header.magic[3]=0x58;	
		mtk_header.size=ramdisk_size;
		memcpy(mtk_header.type,img_cfg.type,32);
		memset(&mtk_header.unused2,0xFF,472);
		if (!mtk_header_write(&mtk_header, fd)) fail("Could not create ramdisk MTK header !");
		if (app_data.verbose) mtk_header_show(&mtk_header);
	}
	
	// Write ramdisk
	output("Repacking ramdisk '%s' (%d bytes)...",ramdisk_file,ramdisk_size);
	if (!repack_file(fd,ramdisk_file)) fail("Could not repack ramdisk '%s' !",ramdisk_file);
	
	// Pad
	if (!pad_file(fd,img_cfg.size-(((kernel_pages+1)*img_header.page_size)+img_header.ramdisk_size),0x0)) fail("Could not pad file !");	
		
	// Closing output file
	fclose(fd);

	if (!file_remove(TMP_RAMDISK_FILENAME)) fail("Could not remove file '%s' !",TMP_RAMDISK_FILENAME);
	
	output("Boot image successfully repacked !");	
	return;
	
failed:
	// Closing input file
	if (fd) fclose(fd);
	// Cleaning outputs	
	if (!file_remove(data->output)) error("Could not remove file '%s' !",data->output);
	if (!file_remove(TMP_RAMDISK_FILENAME)) error("Could not remove file '%s' !",TMP_RAMDISK_FILENAME);	
	exit(1);
}

/**
  * \brief		Repack 'logo.bin' command
  * \param		data 		Repack command data
  */	
void repack_logo(repack_data_t* data)
{
#ifndef LOGO_SUPPORT	
	error("Logo support is not enabled !");
#else
	img_cfg_t		img_cfg;		
	mtk_header_t	mtk_header;	
	FILE*			fd=NULL;
	struct dirent* 	dent = NULL;
	DIR* 			dir;
	char			src[PATH_MAX_SIZE];
	char			dest[PATH_MAX_SIZE];	
	uint32_t		i,logo_count,bloc_size,offset;
	uint32_t*		offsets;
	uint32_t*		sizes;
	
	// Defaulting
	if (!data->compress_rate) data->compress_rate=DEFAULT_COMPRESS_RATE;
	if (!data->output[0]) strcpy(data->output,DEFAULT_LOGO_IMG_FILENAME);	
	if (!data->logos[0]) strcpy(data->logos,DEFAULT_LOGOS_DIR);
	if (!data->config[0]) strcpy(data->config,DEFAULT_CONFIG_FILENAME);		
	
	// Debug parsing/defaulting
	debug("%-20s : %s","verbose",bool2yn(app_data.verbose));
	debug("%-20s : %s","debug",bool2yn(app_data.debug));	
	debug("%-20s : %s","overwrite",bool2yn(data->overwrite));
	debug("%-20s : %s","output",data->output);
	debug("%-20s : %s","logos",data->logos);	
	debug("%-20s : %s","config",data->config);		
	
	// Check input file
	if (!dir_exists(data->logos)) die("Input directory '%s' not found !",data->logos);	
	if (!file_exists(data->config)) die("Input file '%s' not found !",data->config);
	
	// Check for existing output files
	if (!data->overwrite){			
		if (file_exists(data->output)) die("File '%s' already exist !",data->output);
	}
	
	// Clean previous output files
	if (!file_remove(data->output)) die("Could not remove file '%s' !",data->output);
	if (!dir_remove(TMP_LOGO_DIR)) die("Could not remove directory '%s' !",TMP_LOGO_DIR);
	
	// Loading image configuration
	if (!img_cfg_read(&img_cfg, data->config)) die("Could not load image configuration file '%s' !",data->config);	
		
	// Check for config file type for 'LOGO'
	if 	(strcmp(img_cfg.type,"LOGO")) die("Configuration file '%s' is not a valid 'LOGO' image descriptor !",data->config);
		
	// Banner
	output("Repacking logo to '%s'...",data->output);	
	
	// Create temporary directory
	if (!dir_create(TMP_LOGO_DIR)) die("Could not create directory '%s' !",TMP_LOGO_DIR);
	
	// Convert all images
	output("Loading logo from '%s'...",data->logos);
	dir=opendir(data->logos);
	if (dir==NULL) fail("Could not open directory '%s' !",data->logos);
	logo_count=0;
	while ((dent=readdir(dir))!=NULL){
		if ((!strcmp(dent->d_name,"."))||(!strcmp(dent->d_name,".."))) continue;
		sprintf(src,"%s%c%s",data->logos,FILESEP,dent->d_name);		
		if (!is_file(src)) continue;
		if (strlen(dent->d_name)<strlen(LOGOS_FILENAME_PREFIX)) continue;
		if (strncmp(dent->d_name,LOGOS_FILENAME_PREFIX,strlen(LOGOS_FILENAME_PREFIX))) continue;
		verbose("\nRepacking image %d (%s)...",logo_count+1, src);
		// Converting png image to rgb565 if need
		if (!strncmp(&src[strlen(src)-4],".png",4)){
			sprintf(dest,"%s%c%s%02d.rgb565",TMP_LOGO_DIR,FILESEP,LOGOS_FILENAME_PREFIX,(logo_count+1));
			verbose("Converting PNG to RGB565 to '%s'...",dest); 			
			if (!png_to_rgb565(src,dest)) fail("Could not convert logo image '%s' to RGB565 !",src);
			strcpy(src,dest);
		}else if (strncmp(&src[strlen(src)-7],".rgb565",7)){
			fail("Unknown logo image '%s' format !",src);		
		}
		sprintf(dest,"%s%c%s%02d.bin",TMP_LOGO_DIR,FILESEP,LOGOS_FILENAME_PREFIX,(logo_count+1));
		verbose("Deflating image to '%s'...",dest); 
		if (!deflate_file(src,dest,data->compress_rate)) fail("Could not deflate logo image file '%s' !",src);
		logo_count++;
	}
	verbose("");
	closedir(dir);	
	
	// Create map
	verbose("Create offsets map...");
	offsets=malloc(logo_count*sizeof(uint32_t));
	if (!offsets) fail("Could not allocate %d bytes !",logo_count*sizeof(uint32_t));
	sizes=malloc(logo_count*sizeof(uint32_t));
	if (!sizes) fail("Could not allocate %d bytes !",logo_count*sizeof(uint32_t));	
	offset=8+(logo_count*sizeof(uint32_t));
	bloc_size=offset;
	for(i=0;i<logo_count;i++){
		sprintf(src,"%s%c%s%02d.bin",TMP_LOGO_DIR,FILESEP,LOGOS_FILENAME_PREFIX,i+1);		
		offsets[i]=offset;
		sizes[i]=file_size(src);
		bloc_size+=sizes[i];
		offset+=sizes[i];
	}
	if (app_data.verbose){
		verbose("\n    img   |   offset   | size (bytes)");
		verbose(" -------------------------------------");
		for(i=0;i<logo_count;i++){
			verbose("     %.2d   | 0x%08X |   %8d",i+1,offsets[i],sizes[i]);
		}
		verbose(" -------------------------------------");
		verbose("                           %8d\n",bloc_size);		
	}	
	output("%d logo found !",logo_count);
	
	// Openning output file
	fd=fopen(data->output,"wb");
	if (fd==NULL) fail("Could not open output file '%s' !",data->output);	

	// Write MTK Header
	memset(&mtk_header,0x0,sizeof(mtk_header_t));
	mtk_header.magic[0]=0x88; mtk_header.magic[1]=0x16;	mtk_header.magic[2]=0x88; mtk_header.magic[3]=0x58;	
	mtk_header.size=bloc_size;
	strcpy(mtk_header.type,"LOGO");
	memset(&mtk_header.unused2,0xFF,472);
	if (!mtk_header_write(&mtk_header, fd)) fail("Could not create logo MTK header !");
	if (app_data.verbose) mtk_header_show(&mtk_header);
			
	// Offset map
	verbose("Writing offsets map...");	
	// Write logo count
	if (fwrite(&logo_count,1,4,fd)!=4) fail("Could not write logo count !");             
	// Write bloc count
	if (fwrite(&bloc_size,1,4,fd)!=4) fail("Could not write bloc size !");             
	// Write offset map
	if (fwrite(offsets,1,logo_count*sizeof(uint32_t),fd)!=logo_count*sizeof(uint32_t)) fail("Could not write offset map size !");             
		
	// Repack all
	verbose("Writing %d logos...",logo_count);	
	for(i=0;i<logo_count;i++){
		sprintf(src,"%s%c%s%02d.bin",TMP_LOGO_DIR,FILESEP,LOGOS_FILENAME_PREFIX,i+1);
		debug("Repacking '%s'...",src);
		if (!repack_file(fd,src)) fail("Could not repack logo '%s' !",src);
	}
	
	// Pad
	if (!pad_file(fd,img_cfg.size-(bloc_size+sizeof(mtk_header_t)),0x0)) fail("Could not pad file !");
	
	// Cleaning
	if (!dir_remove(TMP_LOGO_DIR)) die("Could not remove directory '%s' !",TMP_LOGO_DIR);
	
	// Closing output file
	fclose(fd);	
	
	output("Logo image successfully repacked !");	
	return;
	
failed:
	// Closing input file
	if (fd) fclose(fd);
	// Cleaning outputs	
	if (!file_remove(data->output)) error("Could not remove file '%s' !",data->output);	
	if (!dir_remove(TMP_LOGO_DIR)) die("Could not remove directory '%s' !",TMP_LOGO_DIR);	
#endif		
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
			if ((!strcmp(args->argv[i],"--type")) || (!strcmp(args->argv[i],"-t"))){
				if ((i+1)>=args->argc) die("Argument needed after option '%s' !",args->argv[i]);
				if (!strcmp(args->argv[i+1],"boot")) data->type=CMD_TYPE_BOOT;
				else if (!strcmp(args->argv[i+1],"logo")) data->type=CMD_TYPE_LOGO;
				else die("Unknown --type '%s' !",args->argv[i+1]);				
				i++;			
			}else if ((!strcmp(args->argv[i],"--overwrite")) || (!strcmp(args->argv[i],"-o"))){
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


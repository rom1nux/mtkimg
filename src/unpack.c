/** ***************************************************************************
  *                          MTKIMG PROJECT
  * ***************************************************************************
  * \brief		Unpack command  
  * \file		unpack.c
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
#include "unpack.h"
 
 /**
 * \brief		Unpack command
 * \param		args 		Command line arguments
 */	 
void unpack(args_t* args)
{ 
	unpack_data_t	data;	
	
	// Init
	memset(&data,0x0,sizeof(unpack_data_t));
	
	// Parse data	
	unpack_parse_args(&data, args);
	
	// Early options check
	if ((data.keep_mtk_header) && (!data.no_decompress)){
		warning("Option '--keep-mtk-header' implies '--no-zip' !");
		data.no_decompress=true;
	}
		
	// Defaulting
	if (!data.input[0])	strcpy(data.input,DEFAULT_IMG_FILENAME);			
	if (!data.config[0]) strcpy(data.config,DEFAULT_CONFIG_FILENAME);	
	if (data.keep_mtk_header){		
		if (!data.kernel[0]) strcpy(data.kernel,DEFAULT_KERNEL_MTK_FILENAME);
		if (!data.ramdisk[0]) strcpy(data.ramdisk,DEFAULT_RAMDISK_MTK_FILENAME);		
	}else{
		if (!data.kernel[0]) strcpy(data.kernel,DEFAULT_KERNEL_FILENAME);		
		if (!data.ramdisk[0]) strcpy(data.ramdisk,( data.no_decompress ? DEFAULT_RAMDISK_FILENAME : DEFAULT_RAMDISK_DIR ));
	}
	
	// Debug parsing/defaulting
	debug("%-20s : %s","verbose",bool2yn(app_data.verbose));
	debug("%-20s : %s","debug",bool2yn(app_data.debug));	
	debug("%-20s : %s","overwrite",bool2yn(data.overwrite));
	debug("%-20s : %s","input",data.input);
	debug("%-20s : %s","kernel",data.kernel);
	debug("%-20s : %s","ramdisk",data.ramdisk);
	debug("%-20s : %s","config",data.config);	
	debug("%-20s : %s","no-decompress",bool2yn(data.no_decompress));	
	debug("%-20s : %s","keep-mtk-header",bool2yn(data.keep_mtk_header));	
		
	// Check input file
	if (!file_exists(data.input)) die("Input file '%s' not found !",data.input);
	
	// Check for MTK file (logo.bin)
	if (is_valid_mtk_file(data.input)){
		// unpack logo.bin
		unpack_logo(&data);
	}else{	
		// Unpack boot.img		
		unpack_boot(&data);
	}
}
 
 
 /**
 * \brief		Unpack 'boot.img' command
 * \param		data 		Unpack command data
 */	
void unpack_boot(unpack_data_t* data)
{
	img_cfg_t		img_cfg;	
	img_header_t	img_header;
	mtk_header_t	mtk_header;		
	FILE*			fs=NULL;	
	uint32_t		kernel_offset, kernel_size, kernel_pages;
	uint32_t		ramdisk_offset, ramdisk_size, ramdisk_pages;
	char*			ramdisk_file;
	char 			syscmd[CMD_MAX_SIZE];
	int				res;	
	
	// Init	
	memset(&img_cfg,0x0,sizeof(img_cfg_t));
	img_cfg.size=file_size(data->input);	
	
	// Check for existing output files
	if (!data->overwrite){			
		if (file_exists(data->kernel)) die("File '%s' already exist !",data->kernel);
		if (data->no_decompress){
			if (file_exists(data->ramdisk)) die("File '%s' already exist !",data->ramdisk);
		}else{
			if (dir_exists(data->ramdisk)) die("Directory '%s' already exist !",data->ramdisk);		
		}
		if (file_exists(data->config)) die("File '%s' already exist !",data->config);		
	}
	
	// Clean previous output files or directory	
	if (!file_remove(TMP_RAMDISK_FILENAME)) die("Could not remove file '%s' !",TMP_RAMDISK_FILENAME);
	if (!file_remove(data->config)) die("Could not remove file '%s' !",data->config);
	if (!file_remove(data->kernel)) die("Could not remove file '%s' !",data->kernel);
	if (data->no_decompress){
		if (!file_remove(data->ramdisk)) die("Could not remove file '%s' !",data->ramdisk);
	}else{
		if (!dir_remove(data->ramdisk)) die("Could not remove directory '%s' !",data->ramdisk);
	}	
	
	// Openning input filename
	fs=fopen(data->input,"rb");
	if (fs==NULL) die("Could not open input file '%s' !",data->input);
	output("Unpacking boot image '%s'...",data->input);
	
	// Reading image header	
	if (!img_header_read(&img_header,fs)) fail("Could not read image header !");	
	if (app_data.debug) img_header_show(&img_header);
	if (!img_header_check_signature(&img_header)) fail("Android image signature is not valid !");
	memcpy(&img_cfg.header,&img_header,sizeof(img_header_t));
	
	// Compute offsets
	kernel_offset=img_header.page_size;
	kernel_size=img_header.kernel_size;
	kernel_pages=(long)ceil(img_header.kernel_size/img_header.page_size)+1;	
	ramdisk_offset=(kernel_pages+1)*img_header.page_size;
	ramdisk_size=img_header.ramdisk_size;
	ramdisk_pages=(long)ceil(img_header.ramdisk_size/img_header.page_size);
	
	// Show layout
	if (app_data.debug){
		printf("\nImage layout :\n\n");
		printf(" %-30s : %d bytes\n","Image size",img_cfg.size);
		printf(" %-30s : 0x%08X\n","Kernel offset",kernel_offset);
		printf(" %-30s : %-20d (%d bytes)\n","Kernel pages",kernel_pages,kernel_pages*img_header.page_size);
		printf(" %-30s : 0x%08X\n","Ramdisk offset",ramdisk_offset);
		printf(" %-30s : %-20d (%d bytes)\n","Ramdisk pages",ramdisk_pages,ramdisk_pages*img_header.page_size);
		putchar('\n');
	}
	
	// KERNEL
	
	// Seeking to start of kernel
	verbose("Seeking to start of kernel (0x%08X)...",kernel_offset); 
	if (fseek(fs,kernel_offset,SEEK_SET)) fail("Could not seek to start of kernel !");
	// Handle MTK header
	if (!data->keep_mtk_header){
		// Reading MTK header		
		if (!mtk_header_read(&mtk_header,fs)) fail("Could not read kernel header !");	
		if (app_data.debug) mtk_header_show(&mtk_header);
		// Checking MTK header
		if (!mtk_header_check_magic(&mtk_header)) fail("Kernel header magic is not valid !");
		if (!mtk_header_check_type(&mtk_header,"KERNEL")) fail("Kernel header type is not valid !");
		if (!mtk_header_check_size(&mtk_header,img_header.kernel_size)) fail("Kernel header size not consistent with image header size !");	
		// Get kernel size without header
		kernel_size=mtk_header.size;
	}
	// Unpacking
	output("Unpacking kernel to '%s' (%d bytes)...",data->kernel,kernel_size); 
	if (!unpack_file(fs,data->kernel,kernel_size)) fail("Could not unpack kernel to file '%s' !",data->kernel);
	// Check for valid ARM Linux zImage
	if (!data->keep_mtk_header){
		if (!is_valid_zimage_file(data->kernel)) fail("Kernel file '%s' is not a valid ARM Linux zImage file !",data->kernel);
	}
	
	// RAMDISK
	
	// Seeking to start of ramdisk
	verbose("Seeking to start of ramdisk (0x%08X)...",ramdisk_offset); 
	if (fseek(fs,ramdisk_offset,SEEK_SET)) fail("Could not seek to start of ramdisk !");
	// Handle MTK header
	if (!data->keep_mtk_header){			
		// Reading MTK header		
		if (!mtk_header_read(&mtk_header,fs)) fail("Could not read ramdisk header !");	
		if (app_data.debug) mtk_header_show(&mtk_header);
		// Checking MTK header
		if (!mtk_header_check_magic(&mtk_header)) fail("Ramdisk header magic is not valid !");
		if ((!mtk_header_check_type(&mtk_header,"ROOTFS")) && (!mtk_header_check_type(&mtk_header,"RECOVERY"))) fail("Ramdisk header type is not valid !");
		if (!mtk_header_check_size(&mtk_header,img_header.ramdisk_size)) fail("Ramdisk header size not consistent with image header size !");				
		// Get ramdisk size without header
		ramdisk_size=mtk_header.size;
		// Get ramdisk type
		memcpy(img_cfg.type,mtk_header.type,32);
	}
	// Unpacking	
	ramdisk_file=(data->no_decompress ? data->ramdisk : TMP_RAMDISK_FILENAME);
	output("Unpacking ramdisk to '%s' (%d bytes)...",ramdisk_file,ramdisk_size); 
	if (!unpack_file(fs,ramdisk_file,ramdisk_size)) fail("Could not unpack ramdisk to file '%s' !",ramdisk_file);
	// Check for valid gzip file
	if (!data->keep_mtk_header){
		if (!is_valid_gzip_file(ramdisk_file)) fail("Ramdisk file '%s' is not a valid gzip file !",ramdisk_file);
	}
	
	// Unpacking ramdisk content
	if (!data->no_decompress){
		output("Unpacking ramdisk content to '%s'...",data->ramdisk); 
		// Create directory
		if (!dir_create(data->ramdisk)) fail("Could not create directory '%s' !",data->ramdisk);
		// Entering directoy
		if (!dir_change(data->ramdisk)) fail("Could change to directory '%s' !",data->ramdisk);
		// Create decompress command line
		sprintf(syscmd,"%s -d -c ..%c%s | %s -i --quiet",GZIP_BIN,FILESEP,TMP_RAMDISK_FILENAME,CPIO_BIN); 			
		// Decompression
		verbose("Unpacking ramdisk content...");
		verbose("%s",syscmd);
		res=system(syscmd);
		// Returning to previous directory
		if (!dir_change(app_data.working_dir)) fail("Could change to directory '%s' !",data->ramdisk);	
		if (res) fail("Could not unpack ramdisk content to directory '%s' !",data->ramdisk);			
		// Remove ramdisk file
		if (!file_remove(TMP_RAMDISK_FILENAME)) fail("Could not remove file '%s' !",TMP_RAMDISK_FILENAME);		
	}
	
	// Writing image configuration		
	if (!img_cfg_write(&img_cfg,data->config)) fail("Could not write image configuration file '%s' !",data->config);
		
	// Closing input filename
	fclose(fs);
	
	output("Boot image successfully unpacked !");	
	return;
	
failed:
	// FAILED
	// Closing input filename
	if (fs) fclose(fs);
	// Cleaning outputs	
	if (!file_remove(data->kernel)) error("Could not remove file '%s' !",data->kernel);
	if (data->no_decompress){
		if (!file_remove(data->ramdisk)) error("Could not remove file '%s' !",data->ramdisk);
	}else{
		if (!dir_remove(data->ramdisk)) error("Could not remove directory '%s' !",data->ramdisk);
	}
	if (!file_remove(data->config)) error("Could not remove file '%s' !",data->config);
	if (!file_remove(TMP_RAMDISK_FILENAME)) error("Could not remove file '%s' !",TMP_RAMDISK_FILENAME);
	exit(1);
}

 /**
 * \brief		Unpack 'logo.bin' command
 * \param		data 		Unpack command data
 */	
void unpack_logo(unpack_data_t* data)
{
	img_cfg_t		img_cfg;	
	mtk_header_t	mtk_header;		
	FILE*			fs=NULL;
	uint32_t		logo_count, bloc_size;
		
	// Init	
	memset(&img_cfg,0x0,sizeof(img_cfg_t));
	img_cfg.size=file_size(data->input);
	
	// Openning input filename
	fs=fopen(data->input,"rb");
	if (fs==NULL) die("Could not open input file '%s' !",data->input);
	output("Unpacking logo image '%s'...",data->input);
	
	// Reading MTK header		
	if (!mtk_header_read(&mtk_header,fs)) fail("Could not read logo header !");	
	if (app_data.debug) mtk_header_show(&mtk_header);
	
	// Checking MTK header
	if (!mtk_header_check_magic(&mtk_header)) fail("Logo header magic is not valid !");
	if (!mtk_header_check_type(&mtk_header,"LOGO")) fail("Logo header type is not valid !");
	//if (!mtk_header_check_size(&mtk_header,img_cfg.size)) fail("Logo header size not consistent with image size !");	
	
	// Reading logo count
	verbose("Reading logo count (%d bytes)...",sizeof(uint32_t));
	if (fread(&logo_count,1,sizeof(uint32_t),fs)!=sizeof(uint32_t)) die("Could not read logo count !");
	output("%d logo found !",logo_count);
	
	// Reading bloc size
	verbose("Reading block size (%d bytes)...",sizeof(uint32_t));
	if (fread(&bloc_size,1,sizeof(uint32_t),fs)!=sizeof(uint32_t)) die("Could not read bloc size !");
	verbose("Bloc size : %d bytes !",bloc_size);
	if (bloc_size!=mtk_header.size) fail("Bloc size is not consistent with MTK header size !");	
	
	// Closing input filename
	fclose(fs);
	
	//output("Logo image successfully unpacked !");	
	warning("Sorry unpack_logo() function is not completed yet !");
	return;
	
failed:
	// FAILED
	// Closing input filename
	if (fs) fclose(fs);	
	exit(1);	
}

/**
 * \brief		Unpack command argument parser
 * \param		data 		Data to fill
 * \param		args 		Arguments to parse
 */	
void unpack_parse_args(unpack_data_t* data, args_t* args)
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
			}else if ((!strcmp(args->argv[i],"--no-decompress")) || (!strcmp(args->argv[i],"-n"))){
				data->no_decompress=true;					
			}else if ((!strcmp(args->argv[i],"--keep-mtk-header")) || (!strcmp(args->argv[i],"-m"))){
				data->keep_mtk_header=true;				
			}else{
				die("Unknown option '%s' (%d) for command '%s' !",args->argv[i],i,CMD_UNPACK);
			}
		}else{
			// Arguments
			if (nargs==1) strcpy(data->input,args->argv[i]);		
			else die("Unknown argument '%s' (%d) !",args->argv[i],i);
			nargs++;
		}
	}
	
}


/** ***************************************************************************
  *                          MTKIMG PROJECT
  * ***************************************************************************
  * \brief		Usefull utilities     
  * \file		tools.c
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
#include "tools.h"

/**
 * \brief		Check for valid ARM Linux zImage file
 * \param		filename		Filename to check
 * \return 		Operation status
 * \retval 		true			Valid
 * \retval 		false			Invalid
 */	
bool is_valid_zimage_file(char* filename)
{
	FILE* fs;
	bool isvalid=false;
	uint8_t buffer[4];
	memset(buffer,0x0,4);
	verbose("Reading kernel zimage header from '%s' (%d bytes)...",filename,4);
	fs=fopen(filename,"rb");
	if (!fs) return false;	
	if (fseek(fs,0x24,SEEK_SET)) return false;	
	isvalid=((fread(&buffer,1,4,fs)==4) && (buffer[0]==0x18) && (buffer[1]==0x28) && (buffer[2]==0x6F) && (buffer[3]==0x01));		
	fclose(fs);
	return isvalid;
}

/**
 * \brief		Check for valid GZIP file
 * \param		filename		Filename to check
 * \return 		Operation status
 * \retval 		true		Valid
 * \retval 		false		Invalid
 */	
bool is_valid_gzip_file(char* filename)
{
	FILE* fs;
	bool isvalid=false;
	uint8_t buffer[2];
	memset(buffer,0x0,2);
	verbose("Reading gzip header from '%s' (%d bytes)...",filename,2);
	fs=fopen(filename,"rb");
	if (!fs) return false;	
	isvalid=((fread(&buffer,1,2,fs)==2) && (buffer[0]==0x1F) && (buffer[1]==0x8B));		
	fclose(fs);
	return isvalid;
}

/**
 * \brief		Check for valid file with valid MTK header
 * \param		filename		Filename to check
 * \return 		Operation status
 * \retval 		true			Valid
 * \retval 		false			Invalid
 */	
bool is_valid_mtk_file(char* filename)
{
	FILE* fs;
	bool isvalid=false;
	uint8_t buffer[4];
	memset(buffer,0x0,4);
	verbose("Reading mtk header from '%s' (%d bytes)...",filename,4);
	fs=fopen(filename,"rb");
	if (!fs) return false;	
	isvalid=((fread(&buffer,1,4,fs)==4) && (buffer[0]==0x88) && (buffer[1]==0x16) && (buffer[2]==0x88) && (buffer[3]==0x58));		
	fclose(fs);
	return isvalid;
}

/**
 * \brief		Write configuration file
 * \param		cfg				Configuation structure to save
 * \param		filename		Filename to write
 * \return 		Operation status
 * \retval 		true			Success
 * \retval 		false			Error
 */
bool img_cfg_write(img_cfg_t* cfg, char* filename)
{
	int i;
	FILE* fd;
	verbose("Writing image configuration file '%s'...",filename);
	fd=fopen(filename,"wt");
	if (!fd) return false;
	fprintf(fd,"; Image configuration file - Write by %s V%s(%s%d)\n",app_data.exename,APP_VERSION,APP_PLATFORM,APP_ARCH);	
	fprintf(fd,"\n; Header\n\n"); 
	fprintf(fd,"signature="); fputnchar(fd,cfg->header.signature,8); putc('\n',fd);
	fprintf(fd,"kernel-load-addr=0x%08X\n",cfg->header.kernel_load_addr);
	fprintf(fd,"ramdisk-load-addr=0x%08X\n",cfg->header.ramdisk_load_addr);
	fprintf(fd,"second-load-addr=0x%08X\n",cfg->header.second_load_addr);
	fprintf(fd,"tags-addr=0x%08X\n",cfg->header.tags_addr);
	fprintf(fd,"page-size=%d\n",cfg->header.page_size);
	fprintf(fd,"product="); fputnchar(fd,cfg->header.product,16); putc('\n',fd);
	fprintf(fd,"cmdline="); fputnchar(fd,cfg->header.cmdline,512); putc('\n',fd);
	fprintf(fd,"id="); for(i=0;i<20;i++) fprintf(fd,"%02X",cfg->header.id[i]); putc('\n',fd);
	fprintf(fd,"\n; Layout\n\n"); 
	fprintf(fd,"size=%d\n",cfg->size);
	fprintf(fd,"type="); fputnchar(fd,cfg->type,8); putc('\n',fd);
	fclose(fd);
	return true;
}

/**
 * \brief		Read configuration file
 * \param		cfg				Configuation structure to fill
 * \param		filename		Filename to read
 * \return 		Operation status
 * \retval 		true			Success
 * \retval 		false			Error
 */
bool img_cfg_read(img_cfg_t* cfg, char* filename)
{
	FILE* fs;
	unsigned int linesz,nline=0;
	char line[BUFFER_SIZE];
	memset(cfg,0x0,sizeof(img_cfg_t));
	verbose("Loading image configuration file '%s'...",filename);	
	fs=fopen(filename,"rt");
	if (!fs) return false;
	while(!feof(fs)){
		line[0]=0;
		if ((!fgets(line,BUFFER_SIZE,fs)) && (!feof(fs))) { error("Could not read line %d !",nline+1); return false; }
		nline++;
		linesz=strlen(line);
		if (!linesz) continue;
		if (line[linesz-1]==10) line[linesz-1]=0;
		if ((line[0]==';') || (line[0]==0)) continue;
		if (!img_cfg_read_line(cfg,nline,line)) return false;
	}
	fclose(fs);
	return true;
}

/**
 * \brief		Decode configuration file line
 * \param		cfg				Configuation structure
 * \param		nline			Line number
 * \param		line			Line buffer 
 * \return 		Operation status
 * \retval 		true			Success
 * \retval 		false			Error
 */
bool img_cfg_read_line(img_cfg_t* cfg, unsigned int nline, char* line)
{
	char* key=line;
	char* value=strchr(line,'=');
	char hexv[3];
	char* ptr;
	int i;
	if (value==NULL){ error("Bad format at line %d !",nline); return false; }
	*value=0; value++;
	if (!strcmp(key,"signature")){
		if (strlen(value)>8){ error("Bad '%s' value at line %d !",key,nline); return false; }	
		memcpy(cfg->header.signature,value,strlen(value));
	}else if (!strcmp(key,"kernel-load-addr")){
		cfg->header.kernel_load_addr=strtoul(value,NULL,0);		 
	}else if (!strcmp(key,"ramdisk-load-addr")){
		cfg->header.ramdisk_load_addr=strtoul(value,NULL,0);	
	}else if (!strcmp(key,"second-load-addr")){
		cfg->header.second_load_addr=strtoul(value,NULL,0);	
	}else if (!strcmp(key,"tags-addr")){
		cfg->header.tags_addr=strtoul(value,NULL,0);	
	}else if (!strcmp(key,"page-size")){
		cfg->header.page_size=strtoul(value,NULL,0);	
	}else if (!strcmp(key,"product")){
		if (strlen(value)>16){ error("Bad '%s' value at line %d !",key,nline); return false; }	
		memcpy(cfg->header.product,value,strlen(value));
	}else if (!strcmp(key,"cmdline")){
		if (strlen(value)>512){ error("Bad '%s' value at line %d !",key,nline); return false; }	
		memcpy(cfg->header.cmdline,value,strlen(value));
	}else if (!strcmp(key,"id")){
		if (strlen(value)!=40){ error("Bad '%s' value at line %d !",key,nline); return false; }		
		ptr=value; hexv[2]=0;
		for(i=0;i<20;i++){
			hexv[0]=*ptr; ptr++;
			hexv[1]=*ptr; ptr++;
			cfg->header.id[i]=strtoul(hexv,NULL,16);
		}
	}else if (!strcmp(key,"size")){
		cfg->size=strtoul(value,NULL,0);	
	}else if (!strcmp(key,"type")){
		if (strlen(value)>32){ error("Bad '%s' value at line %d !",key,nline); return false; }	
		memcpy(cfg->type,value,strlen(value));
	}else{
		 error("Unknown key '%s' at line %d !",key,nline); 
		 return false;
	}
	return true;
}

/**
 * \brief		Show configuration file info
 * \param		cfg				Configuation structure
 */
void img_cfg_show(img_cfg_t* cfg)
{
	img_header_show(&cfg->header);
	printf(" %-30s : %d\n","Size (bytes)",cfg->size);	
	printf(" %-30s : ","Type"); putnchar(cfg->type,32); putchar('\n');
	putchar('\n');	
}

/**
 * \brief		Read image header from file
 * \param		header			Image header structure to fill
 * \param		fs				Source file handle
 * \return 		Operation status
 * \retval 		true			Success
 * \retval 		false			Error
 */
bool img_header_read(img_header_t* header, FILE* fs)
{
	memset(header,0x0,sizeof(img_header_t));
	verbose("Reading image header (%d bytes)...",sizeof(img_header_t)); 
	return (fread(header,1,sizeof(img_header_t),fs)==sizeof(img_header_t));
}

/**
 * \brief		Write image header to file
 * \param		header			Image header structure to write
 * \param		fd				Destination file handle
 * \return 		Operation status
 * \retval 		true			Success
 * \retval 		false			Error
 */
bool img_header_write(img_header_t* header, FILE* fd)
{		
	verbose("Writing image header (%d bytes)...",sizeof(img_header_t));
	return (fwrite(header,1,sizeof(img_header_t),fd)==sizeof(img_header_t));
}

/**
 * \brief		Check image header signature
 * \param		header			Image header structure to check
 * \return 		Operation status
 * \retval 		true			Valid
 * \retval 		false			Invalid
 */
bool img_header_check_signature(img_header_t* header)
{
	return (!strncmp(header->signature,"ANDROID!",8));
}

/**
 * \brief		Show image header info
 * \param		header			Image header structure
 */
void img_header_show(img_header_t* header)
{
	int i;
	printf("\nImage header :\n\n");
	printf(" %-30s : ","Signature"); putnchar(header->signature,8); putchar('\n'); putchar('\n');	
	printf(" %-30s : %d bytes\n","Kernel size",header->kernel_size);
	printf(" %-30s : 0x%08X\n\n","Kernel load address",header->kernel_load_addr);
	printf(" %-30s : %d bytes\n","Ramdisk size",header->ramdisk_size);
	printf(" %-30s : 0x%08X\n\n","Ramdisk load address",header->ramdisk_load_addr);	
	printf(" %-30s : %d bytes\n","Second stage size",header->second_size);
	printf(" %-30s : 0x%08X\n\n","Second stage load address",header->second_load_addr);	
	printf(" %-30s : 0x%08X\n","Tags address",header->tags_addr);
	printf(" %-30s : %d\n","Page size (bytes)",header->page_size);	
	printf(" %-30s : '","Product name"); putnchar(header->product,16); printf("'\n");
	printf(" %-30s : '","Command line"); putnchar(header->cmdline,512); printf("'\n");	
	printf(" %-30s : ","ID"); for(i=0;i<20;i++) printf("%02X",header->id[i]); putchar('\n');
}

/**
 * \brief		Read MTK header from file
 * \param		header			MTK Header structure to fill
 * \param		fs				Source file handle
 * \return 		Operation status
 * \retval 		true			Success
 * \retval 		false			Error
 */	
bool mtk_header_read(mtk_header_t* header, FILE* fs)
{
	memset(header,0x0,sizeof(mtk_header_t));
	verbose("Reading MTK header (%d bytes)...",sizeof(mtk_header_t));
	return (fread(header,1,sizeof(mtk_header_t),fs)==sizeof(mtk_header_t));
}

/**
 * \brief		Write MTK header to file
 * \param		header			MTK Header structure to write
 * \param		fd				Destination file handle
 * \return 		Operation status
 * \retval 		true			Success
 * \retval 		false			Error
 */
bool mtk_header_write(mtk_header_t* header, FILE* fd)
{
	verbose("Writing MTK header (%d bytes)...",sizeof(mtk_header_t));		
	return (fwrite(header,1,sizeof(mtk_header_t),fd)==sizeof(mtk_header_t));
}

/**
 * \brief		Check MTK header magic identifier
 * \param		header			MTK Header structure to check
 * \return 		Operation status
 * \retval 		true			Valid
 * \retval 		false			Invalid
 */
bool mtk_header_check_magic(mtk_header_t* header)
{
	return ((header->magic[0]==0x88) && (header->magic[1]==0x16) && (header->magic[2]==0x88) && (header->magic[3]==0x58)) ;
}

/**
 * \brief		Check MTK header size
 * \param		header			MTK Header structure to check
 * \param		size			Full bloc size
 * \return 		Operation status
 * \retval 		true			Valid
 * \retval 		false			Invalid
 */
bool mtk_header_check_size(mtk_header_t* header, uint32_t size)
{
	return ((header->size+512)==size);
}

/**
 * \brief		Check MTK header type
 * \param		header			MTK Header structure to check
 * \param		type			Type
 * \return 		Operation status
 * \retval 		true		Valid
 * \retval 		false		Invalid
 */
bool mtk_header_check_type(mtk_header_t* header, const char* type)
{
	return (!strncmp(header->type,type,strlen(type)));
}

/**
 * \brief		Show MTK header info
 * \param		header			MTK header structure
 */
void mtk_header_show(mtk_header_t* header)
{
	putchar('\n');	
	printf(" %-30s : 0x%02X 0x%02X 0x%02X 0x%02X\n","Magic",header->magic[0],header->magic[1],header->magic[2],header->magic[3]); 	
	printf(" %-30s : %d\n","Size (bytes)",header->size);
	printf(" %-30s : ","Type"); putnchar(header->type,32); putchar('\n');
	putchar('\n');	
}

/**
 * \brief		Extract file from file
 * \param		fs				Source file handle
 * \param		filename		Destination filename
 * \param		size			Size of block
 * \return 		Operation status
 * \retval 		true			Success
 * \retval 		false			Error
 */
bool unpack_file(FILE *fs, char* filename, unsigned long size)
{
	FILE *fd; 
	char *buffer;
	unsigned long chunksz,nread=0;	
	bool success=true;		
	fd=fopen(filename,"wb");
	if (fd==NULL){ error("Could not open output file '%s' !",filename); return false; }
	buffer=malloc(BUFFER_SIZE);
	if (!buffer){ error("Could not alloc %u bytes !",BUFFER_SIZE); return false; }
	while(nread<size){
		chunksz=(size-nread);
		if (chunksz>BUFFER_SIZE) chunksz=BUFFER_SIZE;
		if (fread(buffer,1,chunksz,fs)!=chunksz){ error("Could not read from file !"); success=false; break; }
		if (fwrite(buffer,1,chunksz,fd)!=chunksz){ error("Could not write to file !"); success=false; break; }
		nread+=chunksz;
	}
	free(buffer);
	fclose(fd);
	return success;
}

/**
 * \brief		Integrate file to file
 * \param		fd				Destination file handle
 * \param		filename		Source filename
 * \return 		Operation status
 * \retval 		true			Success
 * \retval 		false			Error
 */
bool repack_file(FILE *fd, char* filename)
{
	FILE *fs; 
	char *buffer;
	unsigned long nread=0;	
	bool success=true;		
	fs=fopen(filename,"rb");
	if (fs==NULL){ error("Could not open input file '%s' !",filename); return false; }
	buffer=malloc(BUFFER_SIZE);
	if (!buffer){ error("Could not alloc %u bytes !",BUFFER_SIZE); return false; }
	while(!feof(fs)){	
		nread=fread(buffer,1,BUFFER_SIZE,fs);
		if (fwrite(buffer,1,nread,fd)!=nread){ error("Could not write to file !"); success=false; break; }
	}
	free(buffer);
	fclose(fs);
	return success;
}

/**
 * \brief		Padd file
 * \param		fd				Destination file handle
 * \param		size			Size to padd
 * \param		value			Value to fill
 * \return 		Operation status
 * \retval 		true			Success
 * \retval 		false			Error
 */
bool pad_file(FILE *fd, unsigned long size, uint8_t value)
{
	char *buffer;
	unsigned long chunksz,nwrite=0;	
	bool success=true;
	verbose("Padding (%d bytes)...",size);		
	buffer=malloc(BUFFER_SIZE);
	if (!buffer){ error("Could not alloc %u bytes !",BUFFER_SIZE); return false; }
	memset(buffer,value,BUFFER_SIZE);
	while(nwrite<size){
		chunksz=(size-nwrite);
		if (chunksz>BUFFER_SIZE) chunksz=BUFFER_SIZE;
		if (fwrite(buffer,1,chunksz,fd)!=chunksz){ error("Could not write to file !"); success=false; break; }
		nwrite+=chunksz;
	}
	free(buffer);
	return success;
}

/**
 * \brief		Init args manager
 * \param		args			Arguments manager
 */
void args_init(args_t* args)
{
	args->argc=0;
	args->argv=NULL;
}

/**
 * \brief		Free args manager resources
 * \param		args			Arguments manager
 */
void args_free(args_t* args)
{
	int i;
	for(i=0;i<args->argc;i++) free(args->argv[i]);
	free(args->argv);
	args->argc=0;
	args->argv=NULL;	
}

/**
 * \brief		Debug args manager
 * \param		args			Arguments manager
 */
void args_debug(args_t* args)
{
	int i;
	for(i=0;i<args->argc;i++){
		printf("%d > %s\n",i,args->argv[i]);
	}
}

/**
 * \brief		Add to args manager
 * \param		args			Arguments manager
 * \param		arg				Argument to add 
 */
void args_add(args_t* args, char* arg)
{
	size_t 	argsz=strlen(arg)+1;
	if (!args->argc)
		args->argv=malloc(sizeof(char*));
	else
		args->argv=realloc(args->argv,sizeof(char*)*(args->argc+1));
	if (args->argv==NULL) die("Could not allocate %d bytes !",sizeof(char*));
	args->argv[args->argc]=malloc(argsz);
	if (args->argv[args->argc]==NULL) die("Could not allocate %d bytes !",argsz);
	strcpy(args->argv[args->argc],arg);
	args->argc++;	
}

/**
 * \brief		Output message on console
 * \param		fmt		printf like format
 * \param		...		variable arguments
 */	
void output(const char* fmt, ...)
{
	va_list args;	
	va_start(args, fmt); 
	vprintf(fmt,args);
	printf("\n");
	va_end (args);	
}

/**
 * \brief		Output message on console only if verbose is enabled
 * \param		fmt		printf like format
 * \param		...		variable arguments
 */	
void verbose(const char* fmt, ...)
{
	va_list args;	
	if (!app_data.verbose) return;
	va_start(args, fmt); 
	vprintf(fmt,args);
	printf("\n");
	va_end (args);	
}

/**
 * \brief		Output message on console only if debug is enabled
 * \param		fmt		printf like format
 * \param		...		variable arguments
 */	
void debug(const char* fmt, ...)
{
	va_list args;	
	if (!app_data.debug) return;
	va_start(args, fmt); 
	vprintf(fmt,args);
	printf("\n");
	va_end (args);
}

/**
 * \brief		Output WARNING message on console
 * \param		fmt		printf like format
 * \param		...		variable arguments
 */	
void warning(const char* fmt, ...)
{
	va_list args;	
	va_start(args, fmt); 
	printf("WARNING : ");
	vprintf(fmt,args);
	printf("\n");
	va_end (args);	
}

/**
 * \brief		Output ERROR message on console
 * \param		fmt		printf like format
 * \param		...		variable arguments
 */	
void error(const char* fmt, ...)
{
	va_list args;	
	va_start(args, fmt); 
	printf("ERROR : ");
	vprintf(fmt,args);
	printf("\n");
	va_end (args);	
}

/**
 * \brief		Output ERROR message on console and exit application
 * \param		fmt		printf like format
 * \param		...		variable arguments
 */	
void die(const char* fmt, ...)
{
	va_list args;	
	va_start(args, fmt); 
	printf("ERROR : ");
	vprintf(fmt,args);
	printf("\n");
	va_end (args);	
	exit(1);
}

/**
 * \brief		Output nchar character on console
 * \param		str		string to put
 * \param		nchar	Character count
 */	
void putnchar(char* str, int nchar)
{
	unsigned int i;
	for(i=0;i<nchar;i++){
		if (!str[i]) break;
		putchar(str[i]);
	}
}

/**
 * \brief		Output n character to file
 * \param		fd		Destination file handle
 * \param		str		String to put
 * \param		nchar	Character count
 */	
void fputnchar(FILE* fd,char* str, int nchar)
{
	unsigned int i;
	for(i=0;i<nchar;i++){
		if (!str[i]) break;
		fputc(str[i],fd);
	}
}

/**
 * \brief		Check if it's a file
 * \param		filename		Filename to check
 * \return 		Operation status
 * \retval 		true			It's file
 * \retval 		false			It's not a file
 */
bool is_file(char* filename)
{
	struct stat sts;
	if ((stat(filename, &sts) == -1) && (errno == ENOENT)) return 0;
	return S_ISREG(sts.st_mode);
}

/**
 * \brief		Check if file exists
 * \param		filename		Filename to check
 * \return 		Operation status
 * \retval 		true			Exists
 * \retval 		false			Not exists
 */
bool file_exists(char* filename)
{
	struct stat sts;
	return !(stat(filename, &sts) == -1 && errno == ENOENT);	
}

/**
 * \brief		Remove file
 * \param		filename		Filename to remove
 * \return 		Operation status
 * \retval 		true			Success
 * \retval 		false			Error
 */
bool file_remove(char* filename)
{
	if (file_exists(filename)){
		verbose("Removing file '%s'...",filename);
		return (!unlink(filename));
	}
	return true;
}

/**
 * \brief		Get file size in bytes
 * \param		filename		Filename to get size
 * \return 		Size of file 
 */
unsigned long file_size(char* filename)
{
	struct stat sts;
	if (stat(filename, &sts) == -1 && errno == ENOENT) return 0;
	return sts.st_size;
}


/**
 * \brief		Check if it's a directory
 * \param		dirname		Directory to check
 * \return 		Operation status
 * \retval 		true			It's directory
 * \retval 		false			It's not a directory
 */
bool is_dir(char* dirname)
{
	struct stat sts;
	if ((stat(dirname, &sts) == -1) && (errno == ENOENT)) return 0;
	return S_ISDIR(sts.st_mode);
}

/**
 * \brief		Check if directory exists
 * \param		dirname			Directory to check
 * \return 		Operation status
 * \retval 		true			Exists
 * \retval 		false			Not exists
 */
bool dir_exists(char* dirname)
{
	struct stat sts;
	return !(stat(dirname, &sts) == -1 && errno == ENOENT);	
}

/**
 * \brief		Create directory
 * \param		dirname			directory to create
 * \return 		Operation status
 * \retval 		true			Success
 * \retval 		false			Error
 */
bool dir_create(char* dirname)
{
	verbose("Create directory '%s'...",dirname);
#if defined(APP_WIN)	
	return (!mkdir(dirname));
#else
	return (!mkdir(dirname,DIR_CHMOD));
#endif
}

/**
 * \brief		Remove directory
 * \param		dirname		directory to remove
 * \return 		Operation status
 * \retval 		true			Success
 * \retval 		false			Error
 */
bool dir_remove(char* dirname)
{	
	char filename[PATH_MAX_SIZE];	
	struct dirent* dent = NULL;
	DIR* dir;
	if (!dir_exists(dirname)) return true;	
	dir=opendir(dirname);
	if (dir==NULL){ error("Could not open directory '%s' !",dirname); return false;}	
	while ((dent=readdir(dir))!=NULL){
		if ((!strcmp(dent->d_name,"."))||(!strcmp(dent->d_name,".."))) continue;
		sprintf(filename,"%s%c%s",dirname,FILESEP,dent->d_name);
		if (is_dir(filename)){			
			if (!dir_remove(filename)){ error("Could not remove directory '%s' !",filename); break;}
		}else{			
			verbose("Removing file '%s'...",filename);
			if (unlink(filename)){ error("Could not remove file '%s' !",filename); break;}			
		}
	}
	closedir(dir);
	verbose("Removing directory '%s'...",dirname);
	return (!rmdir(dirname));		
}

/**
 * \brief		Change current directory (cd)
 * \param		dirname			directory to go
 * \return 		Operation status
 * \retval 		true			Success
 * \retval 		false			Error
 */
bool dir_change(char* dirname)
{
	verbose("Changing directory '%s'...",dirname);
	return (!chdir(dirname));
}

/**
 * \brief		Get current directory (pwd)
 * \param		dirname			directory buffer to fill
 * \return 		Operation status
 * \retval 		true			Success
 * \retval 		false			Error
 */
bool dir_get_current(char* dirname)
{
	return (!getcwd(dirname,PATH_MAX_SIZE));
}

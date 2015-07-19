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

#ifdef LOGO_SUPPORT

/**
 * \brief		Convert PNG file to RGB565
 * \param		src		Source file
 * \param		dest	Destination file
 * \return 		Operation status
 * \retval 		true			Success
 * \retval 		false			Error
 */	
bool png_to_rgb565(char* src, char *dest)
{
	unsigned char header[8];
	FILE* fs;
	FILE* fd;
	uint8_t* srcbuff;	
	uint8_t* destbuff;	
	uint8_t* srcptr=NULL;
	uint16_t* destptr=NULL;	
	bool success=false;	
	png_structp png_s;
	png_infop png_i;
	uint32_t width, height, x, y, towrite;
	int bit_depth, color_type,npasses;
		
	fs=fopen(src,"rb");
	if (!fs){ error("Could not open file '%s' !",src); goto failed; }
	fd=fopen(dest,"wb");
	if (!fd){ fclose(fs); error("Could not open file '%s' !",dest); goto failed; }
	
	fread(header, 1, 8, fs);
    if (png_sig_cmp(header, 0, 8)){ fclose(fs); fclose(fs); error("File '%s' is not a valid PNG file !",src); goto failed; }
	
	png_s = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);	
	if (!png_s){ fclose(fs); fclose(fs); error("png_create_read_struct failed !"); goto failed; }
	png_i = png_create_info_struct(png_s);
    if (!png_i){ fclose(fs); fclose(fs); error("png_create_info_struct failed !"); goto failed; }
	if (setjmp(png_jmpbuf(png_s))){ fclose(fs); fclose(fs); error("png_init_io failed !"); goto failed; }
	png_init_io(png_s, fs);
	
	png_set_sig_bytes(png_s,8);
		
	png_read_info(png_s, png_i);
	png_get_IHDR(png_s, png_i, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);
	debug("PNG width: %d, height: %d, bit-depth: %d, color-type: %d",width,height,bit_depth,color_type);
		
	if (color_type & PNG_COLOR_MASK_ALPHA) png_set_strip_alpha(png_s);
	if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(png_s);
	if (bit_depth == 16) png_set_strip_16(png_s);	
		
	npasses = png_set_interlace_handling(png_s);
	if (npasses>1) { error("Could not handle interleaved/progressive PNG !"); goto failed; }
		
	png_read_update_info(png_s, png_i);
	
	srcbuff=malloc(width*3);
	if (srcbuff==NULL){ error("Could not allocate %d bytes !",width*3); goto failed; }
	destbuff=malloc(width*2);
	if (destbuff==NULL){ error("Could not allocate %d bytes !",width*2); goto failed; }
		
	towrite=width*2;
	for (y=0;y<height;y++){		
		srcptr=srcbuff;
		destptr=(uint16_t *)destbuff;
		if (setjmp(png_jmpbuf(png_s))){ error("png_read_row failed !"); goto failed; }
		png_read_row(png_s,srcbuff,NULL);		
		for (x=0;x<width;x++){		
			*destptr=((srcptr[0]>>3)<<11) | ((srcptr[1]>>2)<<5) | ((srcptr[2]>>3));		
			srcptr+=3;
			destptr++;
		}		
		if (fwrite(destbuff,1,towrite,fd)!=towrite) { error("Could not write to '%s' !",dest); goto failed; }		
	}
	//png_read_end(png_s, png_i);
	 
	success=true;
failed:
	png_destroy_read_struct(&png_s, &png_i,NULL);
	if (srcbuff) free(srcbuff);
	if (destbuff) free(destbuff);		
	fclose(fd);
	fclose(fs);		
	if (success==false) file_remove(dest);
	return success;	
}

/**
 * \brief		Convert RGB565 file to PNG 
 * \param		src		Source file
 * \param		dest	Destination file
 * \param		width	Image width
 * \param		height	Image height
 * \return 		Operation status
 * \retval 		true			Success
 * \retval 		false			Error
 */	
bool rgb565_to_png(char* src, char *dest, uint32_t width, uint32_t height)
{
	FILE* fs;
	FILE* fd;
	uint32_t i,nread;
	uint8_t* srcbuff;	
	uint8_t* destbuff;	
	uint16_t* srcptr=NULL;
	uint8_t* destptr=NULL;
	bool success=false;		
	png_structp png_s;
	png_infop png_i;
	
	srcbuff=malloc(width*2);
	if (srcbuff==NULL){ error("Could not allocate %d bytes !",width*2); return false; }
	destbuff=malloc(width*3);
	if (destbuff==NULL){ error("Could not allocate %d bytes !",width*3); return false; }	
	
	fs=fopen(src,"rb");
	if (!fs){ error("Could not open file '%s' !",src); goto failed; }
	fd=fopen(dest,"wb");
	if (!fd){ fclose(fs); error("Could not open file '%s' !",dest); goto failed; }
	
	png_s = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_s){ fclose(fs); fclose(fs); error("png_create_write_struct failed !"); goto failed; }
	png_i = png_create_info_struct(png_s);
    if (!png_i){ fclose(fs); fclose(fs); error("png_create_info_struct failed !"); goto failed; }
    if (setjmp(png_jmpbuf(png_s))){ fclose(fs); fclose(fs); error("png_init_io failed !"); goto failed; }
    png_init_io(png_s, fd);            
	
	if (setjmp(png_jmpbuf(png_s))){ fclose(fs); fclose(fs); error("png_set_IHDR failed !"); goto failed; }
    png_set_IHDR(png_s, png_i, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	if (setjmp(png_jmpbuf(png_s))){ fclose(fs); fclose(fs); error("png_write_info failed !"); goto failed; }
    png_write_info(png_s, png_i);

	while(!feof(fs)){
		nread=fread(srcbuff,2,width,fs);
		if ((nread!=width) && (nread!=0)){ error("Could not read from '%s' !",src); goto failed; }
		srcptr=(uint16_t *)srcbuff;
		destptr=destbuff;
		for(i=0;i<nread;i++){		
			destptr[0] = (uint8_t)((*srcptr & 0xF800) >> 11)<<3;
			destptr[1] = (uint8_t)((*srcptr & 0x07E0) >> 5)<<2;
			destptr[2] = (uint8_t)(*srcptr & 0x001F)<<3;				
			srcptr++;
			destptr+=3;
		}		
		if (setjmp(png_jmpbuf(png_s))){ fclose(fs); fclose(fs); error("png_write_row failed !"); goto failed; }
		png_write_row(png_s, destbuff);
	}
	
	if (setjmp(png_jmpbuf(png_s))){ error("png_write_end failed !"); goto failed; }
	png_write_end(png_s, NULL);
	success=true;
		
failed:
	png_destroy_write_struct(&png_s, &png_i);
	if (srcbuff) free(srcbuff);	
	if (destbuff) free(destbuff);			
	fclose(fd);
	fclose(fs);		
	if (success==false) file_remove(dest);
	return success;

}

/**
 * \brief		Compress file
 * \param		src		Source file
 * \param		dest	Destination file
 * \return 		Operation status
 * \retval 		true			Success
 * \retval 		false			Error
 */	
bool deflate_file(char* src, char *dest, int rate)
{
	z_stream strm;
	FILE* fs;
	FILE* fd;
	uint8_t srcbuff[BUFFER_SIZE];
	uint8_t destbuff[BUFFER_SIZE];
	unsigned int towrite, flag;
	bool sucess=true;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    if (deflateInit(&strm,rate) != Z_OK){ error("Could not init zlib !"); return false; }
	fs=fopen(src,"rb");
	if (!fs){ error("Could not open file '%s' !",src); return false; }
	fd=fopen(dest,"wb");
	if (!fd){ fclose(fs); error("Could not open file '%s' !",dest); return false; }
	
	while(!feof(fs)){
		strm.avail_in = fread(srcbuff,1,BUFFER_SIZE,fs);
		strm.next_in = srcbuff;
		do{
            strm.avail_out=BUFFER_SIZE;
            strm.next_out=destbuff;
			flag=(feof(fs) ? Z_FINISH : Z_NO_FLUSH);
            if (deflate(&strm, flag)==Z_STREAM_ERROR){ sucess=false; error("Error during deflate !"); break; }
            towrite=BUFFER_SIZE-strm.avail_out;
            if (fwrite(destbuff,1,towrite,fd)!=towrite){ sucess=false; error("Could not write to '%s' file !",dest); break; }               
        }while (strm.avail_out == 0);
	}		
	
	deflateEnd(&strm);
	fclose(fd);
	fclose(fs);
	return sucess;	
}

/**
 * \brief		Decompress file
 * \param		src		Source file
 * \param		dest	Destination file
 * \return 		Operation status
 * \retval 		true			Success
 * \retval 		false			Error
 */	
bool inflate_file(char* src, char *dest)
{
	z_stream strm;
	FILE* fs;
	FILE* fd;
	uint8_t srcbuff[BUFFER_SIZE];
	uint8_t destbuff[BUFFER_SIZE];
	unsigned int towrite, flag;
	bool sucess=true;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    if (inflateInit(&strm) != Z_OK){ error("Could not init zlib !"); return false; }
	fs=fopen(src,"rb");
	if (!fs){ error("Could not open file '%s' !",src); return false; }
	fd=fopen(dest,"wb");
	if (!fd){ fclose(fs); error("Could not open file '%s' !",dest); return false; }
	while(!feof(fs)){
		strm.avail_in = fread(srcbuff,1,BUFFER_SIZE,fs);
		strm.next_in = srcbuff;
		do{
            strm.avail_out=BUFFER_SIZE;
            strm.next_out=destbuff;
			flag=(feof(fs) ? Z_FINISH : Z_NO_FLUSH);
            if (inflate(&strm, flag)==Z_STREAM_ERROR){ sucess=false; error("Error during inflate !"); break; }
            towrite=BUFFER_SIZE-strm.avail_out;
            if (fwrite(destbuff,1,towrite,fd)!=towrite){ sucess=false; error("Could not write to '%s' file !",dest); break; }               
        }while (strm.avail_out == 0);
	}	
	deflateEnd(&strm);
	fclose(fd);
	fclose(fs);
	return sucess;
}
#endif

/**
 * \brief		Initialize logo database
 * \param		db				Logo database
 * \return 		Operation status
 * \retval 		true			Success
 * \retval 		false			Error
 */
bool logo_db_init(logo_db_t* db)
{
	db->nitems=0;
	db->items=NULL;	
	// Load file if exist
	if (file_exists(LOGO_DB_FILENAME))	return logo_db_read(db,LOGO_DB_FILENAME);
	// Init database
	
	// [Video Graphics Array]
	logo_db_add_comment(db,"; Video Graphics Array");
	// QQVGA
	logo_db_add(db,"QQVGA",		160,	120);	// 4:3	
	// HQVGA
	logo_db_add(db,"HQVGA",		240,	160);	// 3:2	
	// QVGA
	logo_db_add(db,"QVGA",		320,	240);	// 4:3	
	// WQVGA
	logo_db_add(db,"WQVGA",		360,	240); 	// 3:2
	logo_db_add(db,"WQVGA",		376,	240); 	// 4.7:3
	logo_db_add(db,"WQVGA",		384,	240);	// 16:10
	logo_db_add(db,"WQVGA",		400,	240);	// 5:3
	logo_db_add(db,"WQVGA",		428,	240);	// 16:9
	logo_db_add(db,"WQVGA",		432,	240);	// 16:9
	//logo_db_add(db,"WQVGA",	480,	270);	// 16:9		=> Already define in HVGA
	//logo_db_add(db,"WQVGA",	480,	272);	// 16:9		=> Already define in HVGA
	// HVGA
	logo_db_add(db,"HVGA",		480,	270);	// 16:9
	logo_db_add(db,"HVGA",		480,	272);	// 16:9
	logo_db_add(db,"HVGA",		480,	320);	// 3:2	
	logo_db_add(db,"HVGA",		640,	240);	// 8:3	
	logo_db_add(db,"HVGA",		480,	360);	// 4:3	
	// VGA
	logo_db_add(db,"VGA",		640,	480);	// 4:3
	// WVGA
	//logo_db_add(db,"WVGA",	640,	360);	// 16:9		=> Already define in nHD
	logo_db_add(db,"WVGA",		640,	384);	// 5:3
	logo_db_add(db,"WVGA",		720,	480);	// 15:10
	logo_db_add(db,"WVGA",		768,	480);	// 16:10
	logo_db_add(db,"WVGA",		800,	480);	// 5:3
	logo_db_add(db,"WVGA",		848,	480);	// 16:9
	logo_db_add(db,"WVGA",		852,	480);	// 16:9
	logo_db_add(db,"WVGA",		853,	480);	// 16:9
	//logo_db_add(db,"WVGA",	854,	480);	// 16:9		=> Already define in FWVGA
	// FWVGA
	logo_db_add(db,"FWVGA",		854,	480);	// 16:9
	// SVGA
	logo_db_add(db,"SVGA",		800,	600);	// 4:3
	// DVGA
	logo_db_add(db,"DVGA",		960,	640);	// 3:2
	// WSVGA
	logo_db_add(db,"WSVGA",		1024,	576);	// 16:9
	logo_db_add(db,"WSVGA",		1024,	600);	// 15:9
	
	// [Extended Graphics Array]
	logo_db_add_comment(db,"; Extended Graphics Array");
	// XGA
	logo_db_add(db,"XGA",		1024,	768);	// 4:3
	// WXGA
	logo_db_add(db,"WXGA",		1152,	768);	// 15:10
	//logo_db_add(db,"WXGA",	1280,	720);	// 16:9		=> Already define in HD
	logo_db_add(db,"WXGA",		1280,	768);	// 5:3
	logo_db_add(db,"WXGA",		1280,	800);	// 16:10
	logo_db_add(db,"WXGA",		1360,	768);	// 16:9
	//logo_db_add(db,"WXGA",	1366,	768);	// 16:9		=> Already define in FWXGA
	// FWXGA
	logo_db_add(db,"FWXGA",		1366,	768);	// 16:9
	// XGA+
	logo_db_add(db,"XGA+",		1152,	864);	// 4:3
	logo_db_add(db,"XGA+",		1152,	900);	// 1.28:1
	logo_db_add(db,"XGA+",		1152,	870);	// 1.32:1
	logo_db_add(db,"XGA+",		1120,	832);	// 11:8
	// WXGA+
	logo_db_add(db,"WXGA+",		1440,	900);	// 16:10
	// WSXGA
	logo_db_add(db,"WSXGA",		1440,	960);	// 3:2
	// SXGA
	logo_db_add(db,"SXGA",		1280,	1024);	// 5:4
	// SXGA-
	logo_db_add(db,"SXGA-",		1280,	960);	// 4:3
	// SXGA+
	logo_db_add(db,"SXGA+",		1400,	1050);	// 4:3	
	// WSXGA+
	logo_db_add(db,"WSXGA",		1680,	1050);	// 16:10
	// UXGA
	logo_db_add(db,"UXGA",		1600,	1200);	// 4:3
	// WUXGA
	logo_db_add(db,"WUXGA",		1920,	1200);	// 16:10

	// [Quad Extended Graphics Array]
	logo_db_add_comment(db,"; Quad Extended Graphics Array");
	// QWXGA
	logo_db_add(db,"QWXGA",		2048,	1152);	// 16:9
	// QXGA
	logo_db_add(db,"QXGA",		2048,	1536);	// 4:3
	// WQXGA
	logo_db_add(db,"WQXGA",		2560,	1600);	// 16:10
	logo_db_add(db,"WQXGA",		2880,	1600);	// 16:10
	// QSXGA
	logo_db_add(db,"QSXGA",		2560,	2048);	// 5:4
	// WQSXGA
	logo_db_add(db,"WQSXGA",	3200,	2048);	// 25:16
	// QUXGA
	logo_db_add(db,"QUXGA",		3200,	2400);	// 4:3
	// WQUXGA
	logo_db_add(db,"WQUXGA",	3840,	2400);	// 16:10
	
	// [Hyper Extended Graphics Array]
	logo_db_add_comment(db,"; Hyper Extended Graphics Array");
	// HXGA
	logo_db_add(db,"HXGA",		4096,	3072);	// 4:3
	// WHXGA
	logo_db_add(db,"WHXGA",		5120,	3200);	// 16:10
	// HSXGA
	logo_db_add(db,"HSXGA",		5120,	4096);	// 5:4
	// WHSXGA
	logo_db_add(db,"WHSXGA",	6400,	4096);	// 25:16
	// HUXGA
	logo_db_add(db,"HUXGA",		6400,	4800);	// 4:3
	// WHUXGA
	logo_db_add(db,"WHUXGA",	7680,	4800);	// 16:10
	
	// [High Definition]
	logo_db_add_comment(db,"; High Definition");
	// nHD
	logo_db_add(db,"nHD",		640,	360);	// 16:9
	// qHD
	logo_db_add(db,"qHD",		960,	540);	// 16:9
	// HD
	logo_db_add(db,"HD",		1280,	720);	// 16:9
	// HD+
	logo_db_add(db,"HD+",		1600,	900);	// 16:9
	// FHD
	logo_db_add(db,"FHD",		1920,	1080);	// 16:9
	// QHD
	logo_db_add(db,"QHD",		2560,	1440);	// 16:9
	// WQXGA+
	logo_db_add(db,"WQXGA+",	3200,	1800);	// 16:9
	// UHD
	logo_db_add(db,"UHD",		3840,	2160);	// 16:9
	// UHD+
	logo_db_add(db,"UHD+",		5120,	2880);	// 16:9
	// FUHD
	logo_db_add(db,"FUHD",		7680,	4320);	// 16:9
	// QUHD
	logo_db_add(db,"QUHD",		15360,	8640);	// 16:9
	
	// [Miscellaneous]
	logo_db_add_comment(db,"; Miscellaneous");
	logo_db_add(db,"MISC",		54,		38);
	logo_db_add(db,"MISC",		54,		48);
	logo_db_add(db,"MISC",		24,		135);
	logo_db_add(db,"MISC",		1,		135);
		
	// Write file
	return logo_db_write(db,LOGO_DB_FILENAME);
}

/**
 * \brief		Add resolution to logo database
 * \param		db				Logo database
 * \param		label			Resolution label
 * \param		width			Resolution width
 * \param		height			Resolution height
 * \return 		Operation status
 * \retval 		true			Success
 * \retval 		false			Error
 */
bool logo_db_add(logo_db_t* db, char* label, unsigned int width, unsigned int height)
{
	if ((width>0) && (height>0) && (logo_db_exist(db,width,height))){ warning("Resolution %d x %d already exist in database !",width,height); return true;}
	size_t labelsz=strlen(label)+1;
	if (!db->nitems)
		db->items=malloc(sizeof(logo_db_item_t));
	else
		db->items=realloc(db->items,sizeof(logo_db_item_t)*(db->nitems+1));
	if (db->items==NULL){ error("Could not allocate %d bytes !",sizeof(logo_db_item_t)); return false; }
	db->items[db->nitems].label=malloc(labelsz);
	if (db->items[db->nitems].label==NULL){ error("Could not allocate %d bytes !",labelsz); return false; }
	strcpy(db->items[db->nitems].label,label);	
	db->items[db->nitems].width=width;
	db->items[db->nitems].height=height;
	db->nitems++;
	return true;
}

/**
 * \brief		Check if resolution already exist in database
 * \param		db				Logo database
 * \param		width			Resolution width
 * \param		height			Resolution height
 * \return 		Operation status
 * \retval 		true			Exist
 * \retval 		false			Not exist
 */
bool logo_db_exist(logo_db_t* db, unsigned int width, unsigned int height)
{
	unsigned int i;
	for(i=0;i<db->nitems;i++){
		if ((db->items[i].width==width) && (db->items[i].height==height)) return true;
	}
	return false;
}

/**
 * \brief		Clear logo database
 * \param		db				Logo database
 * \return 		Operation status
 * \retval 		true			Success
 * \retval 		false			Error
 */
bool logo_db_clear(logo_db_t* db)
{
	unsigned int i;
	for(i=0;i<db->nitems;i++) free(db->items[i].label);
	if (db->nitems) free(db->items);
	db->nitems=0;
	db->items=NULL;
	return true;
}

/**
 * \brief		Write logo database to file
 * \param		db				Logo database
 * \param		filename		Filename to write database 
 * \return 		Operation status
 * \retval 		true			Success
 * \retval 		false			Error
 */
bool logo_db_write(logo_db_t* db, char* filename)
{
	unsigned int i;
	FILE* fd;
	verbose("Writing logo database to file '%s'...",filename);
	fd=fopen(filename,"wt");
	if (!fd) return false;
	fprintf(fd,"; Logo database - Write by %s V%s(%s%d)\n",app_data.exename,APP_VERSION,APP_PLATFORM,APP_ARCH);	
	fprintf(fd,"; (Source from https://en.wikipedia.org/wiki/Graphics_display_resolution )\n");
	fprintf(fd,"; Format : <label>=<width> x <height>\n");
	for(i=0;i<db->nitems;i++){
		if (db->items[i].label[0]==';'){
			fprintf(fd,"\n%s\n",db->items[i].label);
		}else{
			fprintf(fd,"%s=%d x %d\n",db->items[i].label,db->items[i].width,db->items[i].height);
		}
	}
	fclose(fd);
	return true;
}

/**
 * \brief		Read logo database from file
 * \param		db				Logo database
 * \param		filename		Filename to read database 
 * \return 		Operation status
 * \retval 		true			Success
 * \retval 		false			Error
 */
bool logo_db_read(logo_db_t* db, char* filename)
{	
	FILE* fs;
	unsigned int linesz,nline=0;
	char line[BUFFER_SIZE];
	char* label;
	char* width;
	char* height;
	logo_db_clear(db);
	verbose("Loading logo database from file '%s'...",filename);
	fs=fopen(filename,"rt");
	if (!fs) return false;
	while(!feof(fs)){
		line[0]=0;
		if ((!fgets(line,BUFFER_SIZE,fs)) && (!feof(fs))){ error("Could not read line %d !",nline+1); return false; }
		nline++;
		linesz=strlen(line);
		if (!linesz) continue;
		if (line[linesz-1]==10) line[linesz-1]=0;
		if ((line[0]==';') || (line[0]==0)) continue;
		// Parse line 
		label=line;
		width=strchr(line,'=');
		if (width==NULL){ error("Bad format at line %d !",nline); return false; }
		*width=0; width++;
		height=strchr(width,'x');		
		if (height==NULL){ error("Bad format at line %d !",nline); return false; }
		*height=0; height++;
		// Adding		
		if (!logo_db_add(db,label,strtoul(width,NULL,0), strtoul(height,NULL,0))){ error("Could not add size from line %d !",nline+1); return false; }
		
	}
	fclose(fs);
	return true;
}

/**
 * \brief		Search resolution by pixels count (Width x Height)
 * \param		db				Logo database
 * \param		npixels			Pixels count
 * \param		label			Label of resolution found
 * \param		width			Width of resolution found
 * \param		height			Height of resolution found
 * \return 		Operation status
 * \retval 		true			Success
 * \retval 		false			Error
 */
bool logo_db_find(logo_db_t* db, unsigned int npixels, char* label, unsigned int* width, unsigned int* height)
{
	unsigned int i;
	for(i=0;i<db->nitems;i++){
		if (db->items[i].label[0]==';') continue;
		if ((db->items[i].width*db->items[i].height)==npixels){
			strcpy(label,db->items[i].label);
			*width=db->items[i].width;
			*height=db->items[i].height;
			return true;
		}
	}
	return false;
}

/**
 * \brief		Show database content
 * \param		db				Logo database
 * \return 		Operation status
 * \retval 		true			Success
 * \retval 		false			Error
 */
void logo_db_debug(logo_db_t* db)
{
	unsigned int i;	
	debug("\n -------------------------");
	debug("  label  | width | height");
	debug(" -------------------------");
	for(i=0;i<db->nitems;i++){
		debug("  %-6s | %5d | %5d",db->items[i].label,db->items[i].width,db->items[i].height);
	}
	debug(" -------------------------");
	debug("               %3d items\n\n",db->nitems);
}


/**
 * \brief		Convert command type to string
 * \param		type			Command type
 * \return 		String representation of type 
 */	
const char* cmd_type_to_str(cmd_type_t type)
{
	switch(type){		
		case CMD_TYPE_UNKNOWN: 	return "unknown"; 	break;
		case CMD_TYPE_BOOT: 	return "boot"; 		break;
		case CMD_TYPE_LOGO: 	return "logo"; 		break;
	}
	return "???"; 	
}

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
	fprintf(fd,"\n; Layout\n\n"); 
	fprintf(fd,"size=%d\n",cfg->size);
	fprintf(fd,"type="); fputnchar(fd,cfg->type,32); putc('\n',fd);
	fprintf(fd,"\n; Header (Only used by ROOTFS or RECOVERY image type)\n\n"); 
	fprintf(fd,"signature="); fputnchar(fd,cfg->header.signature,8); putc('\n',fd);
	fprintf(fd,"kernel-load-addr=0x%08X\n",cfg->header.kernel_load_addr);
	fprintf(fd,"ramdisk-load-addr=0x%08X\n",cfg->header.ramdisk_load_addr);
	fprintf(fd,"second-load-addr=0x%08X\n",cfg->header.second_load_addr);
	fprintf(fd,"tags-addr=0x%08X\n",cfg->header.tags_addr);
	fprintf(fd,"page-size=%d\n",cfg->header.page_size);
	fprintf(fd,"product="); fputnchar(fd,cfg->header.product,16); putc('\n',fd);
	fprintf(fd,"cmdline="); fputnchar(fd,cfg->header.cmdline,512); putc('\n',fd);
	fprintf(fd,"id="); for(i=0;i<20;i++) fprintf(fd,"%02X",cfg->header.id[i]); putc('\n',fd);		
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
	printf(" %-30s : %d bytes\n","Page size",header->page_size);	
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
	if (args->argc) free(args->argv);
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
		debug("Removing file '%s'...",filename);
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
	debug("Create directory '%s'...",dirname);
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
			debug("Removing file '%s'...",filename);
			if (unlink(filename)){ error("Could not remove file '%s' !",filename); break;}			
		}
	}
	closedir(dir);
	debug("Removing directory '%s'...",dirname);
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

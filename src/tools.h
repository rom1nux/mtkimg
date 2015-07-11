/** ***************************************************************************
  *                          MTKIMG PROJECT
  * ***************************************************************************
  * \brief		Usefull utilities header   
  * \file		tools.h
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
#ifndef TOOLS_H
#define TOOLS_H

// Standard includes
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>	
#include <unistd.h>
#include <dirent.h>

// Includes
#include "main.h"

// Fies recognition
bool is_valid_zimage_file(char* filename);
bool is_valid_gzip_file(char* filename);
bool is_valid_mtk_file(char* filename);

// Configuration file management
bool img_cfg_write(img_cfg_t* cfg, char* filename);
bool img_cfg_read(img_cfg_t* cfg, char* filename);
bool img_cfg_read_line(img_cfg_t* cfg, unsigned int nline, char* line);
void img_cfg_show(img_cfg_t* cfg);

// Image header managent
bool img_header_read(img_header_t* header, FILE* fs);
bool img_header_write(img_header_t* header, FILE* fd);
bool img_header_check_signature(img_header_t* header);
void img_header_show(img_header_t* header);

// MTK header management
bool mtk_header_read(mtk_header_t* header, FILE* fs);
bool mtk_header_write(mtk_header_t* header, FILE* fd);
bool mtk_header_check_magic(mtk_header_t* header);
bool mtk_header_check_size(mtk_header_t* header, uint32_t size);
bool mtk_header_check_type(mtk_header_t* header, const char* type);
void mtk_header_show(mtk_header_t* header);

// In-file utilites
bool unpack_file(FILE *fs, char* filename, unsigned long size);
bool repack_file(FILE *fd, char* filename);
bool pad_file(FILE *fd, unsigned long size, uint8_t value);

// Arguments management
void args_init(args_t* args);
void args_free(args_t* args);
void args_debug(args_t* args);
void args_add(args_t* args, char *arg);

// Output
void output(const char* fmt, ...);
void verbose(const char* fmt, ...);
void debug(const char* fmt, ...);
void warning(const char* fmt, ...);
void error(const char* fmt, ...);
void die(const char* fmt, ...);

/**
 * \brief		Print error message and goto 'failed' label
 * \param		fmt		printf like format
 * \param		...		variable arguments
 */	
#define fail(fmt, ...) ({error(fmt,##__VA_ARGS__); goto failed;})

void putnchar(char* str, int nchar);
void fputnchar(FILE* fd,char* str, int nchar);

// File
bool is_file(char* filename);
bool file_exists(char* filename);
bool file_remove(char* filename);
unsigned long file_size(char* filename);

// Directory
bool is_dir(char* dirname);
bool dir_exists(char* dirname);
bool dir_create(char* dirname);
bool dir_remove(char* dirname);
bool dir_change(char* dirname);
bool dir_get_current(char* dirname);

#endif
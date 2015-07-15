/** ***************************************************************************
  *                          MTKIMG PROJECT
  * ***************************************************************************
  * \brief		Unpack command header  
  * \file		unpack.h
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
#ifndef UNPACK_H
#define UNPACK_H

// Standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Includes
#include "main.h"
#include "tools.h"

/**
 * \brief		Unpack command data
 */
typedef struct unpack_data_t{
	cmd_type_t		type;							//!< Type of command
	char			input[PATH_MAX_SIZE];			//!< Input file (ex: boot.img)
	bool			overwrite;						//!< Can overwrite output file/directory
	char			kernel[PATH_MAX_SIZE];			//!< Kernel output filename
	char			ramdisk[PATH_MAX_SIZE];			//!< Ramdisk output filename/directory
	char			config[PATH_MAX_SIZE];			//!< Configuration output filename
	char			logos[PATH_MAX_SIZE];			//!< Logos output directory
	bool			no_decompress;					//!< Do not decompress ramdisk
	bool			keep_mtk_header;				//!< Keep MTK head on kernel and ramdisk files
	bool			flip_logo_size;					//!< Reverse the logo size (height x width to width x height)
}unpack_data_t;

// Prototypes
void unpack(args_t* args);
void unpack_boot(unpack_data_t* data);
void unpack_logo(unpack_data_t* data);
void unpack_parse_args(unpack_data_t* data, args_t* args);


#endif
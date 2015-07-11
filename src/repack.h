/** ***************************************************************************
  *                          MTKIMG PROJECT
  * ***************************************************************************
  * \brief		Repack command header   
  * \file		repack.h
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
#ifndef REPACK_H
#define REPACK_H

// Standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Includes
#include "main.h"
#include "tools.h"

/**
 * \brief		Repack command data
 */
typedef struct repack_data_t{
	char			output[PATH_MAX_SIZE];		//!< Output file (ex: boot.img)
	bool			overwrite;					//!< Can overwrite output file
	char			kernel[PATH_MAX_SIZE];		//!< Kernel input filename
	char			ramdisk[PATH_MAX_SIZE];		//!< Ramdisk input filename/directory
	char			config[PATH_MAX_SIZE];		//!< Configuration input filename
	bool			no_compress;				//!< Do not compress ramdisk, use ramdisk as file instread
	int				compress_rate;				//!< gzip compression rate (0-9)
}repack_data_t;

// Prototypes
void repack(args_t* args);
void repack_parse_args(repack_data_t* data, args_t* args);

#endif
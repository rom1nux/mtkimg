/** ***************************************************************************
  *                          MTKIMG PROJECT
  * ***************************************************************************
  * \file		repack.h
  * \author		rom1nux
  * \version	1.0
  * \date		2015-07-09
  * \brief		Repack command header
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
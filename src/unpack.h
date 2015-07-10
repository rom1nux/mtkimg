/** ***************************************************************************
  *                          MTKIMG PROJECT
  * ***************************************************************************
  * \file		unpack.h
  * \author		rom1nux
  * \version	1.0
  * \date		2015-07-09
  * \brief		Unpack command header
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
	char			input[PATH_MAX_SIZE];			//!< Input file (ex: boot.img)
	bool			overwrite;						//!< Can overwrite output file/directory
	char			kernel[PATH_MAX_SIZE];			//!< Kernel output filename
	char			ramdisk[PATH_MAX_SIZE];			//!< Ramdisk output filename/directory
	char			config[PATH_MAX_SIZE];			//!< Configuration output filename
	bool			no_decompress;					//!< Do not decompress ramdisk
	bool			keep_mtk_header;				//!< Keep MTK head on kernel and ramdisk files
}unpack_data_t;

// Prototypes
void unpack(args_t* args);
void unpack_parse_args(unpack_data_t* data, args_t* args);


#endif
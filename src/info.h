/** ***************************************************************************
  *                          MTKIMG PROJECT
  * ***************************************************************************
  * \file		info.h
  * \author		rom1nux
  * \version	1.0
  * \date		2015-07-09
  * \brief		Info command header
  * ***************************************************************************  
  */ 
#ifndef INFO_H
#define INFO_H

// Standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Includes
#include "main.h"
#include "tools.h"

/**
 * \brief		Info command data
 */
typedef struct info_data_t{
	char			input[PATH_MAX_SIZE];		//!< Input file (ex: boot.img)
}info_data_t;

// Prototypes
void info(args_t* args);
void info_parse_args(info_data_t* data, args_t* args);


#endif
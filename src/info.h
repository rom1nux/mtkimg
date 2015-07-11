/** ***************************************************************************
  *                          MTKIMG PROJECT
  * ***************************************************************************
  * \brief		Info command header  
  * \file		info.h
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
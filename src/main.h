/** ***************************************************************************
  *                          MTKIMG PROJECT
  * ***************************************************************************
  * \brief		Main file header  
  * \file		main.h
  * \author		rom1nux
  * \date		2015-07-11
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
  * Note for porting :
  *  - Add/correct platform detection mechanism after "Detect platform" comment
  *  - Add/correct arch detection mechanism after "Detect architecture" comment
  *  - Add/correct platform specificities after "Platform specifics" comment
  * ***************************************************************************    
  */ 
#ifndef MAIN_H
#define MAIN_H

// Standard include
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

// Application constantes
#define	APP_TITLE						"MTKIMG"				//!< Application title
#define	APP_NAME						"mtkimg"				//!< Application name
#define	APP_VERSION						"0.43"					//!< Application version
#define	APP_AUTHOR						"rom1nux"				//!< Application author


// Detect platform
#if defined(__WIN32__) || defined(__WIN64__)
	#define	APP_PLATFORM				"win"					//!< Application platform name
	#define	APP_WIN						1						//!< Application type
#elif defined(__CYGWIN__)
	#define	APP_PLATFORM				"cygwin"				//!< Application platform name
	#define	APP_CYGWIN					1						//!< Application type
#elif defined(__APPLE__) || defined(__MACH__)
	#define	APP_PLATFORM				"osx"					//!< Application platform name
	#define	APP_OSX						1						//!< Application type	
#elif defined(__linux__)
	#define	APP_PLATFORM				"linux"					//!< Application platform name
	#define	APP_LINUX					1						//!< Application type
#else
	#warning "Unsupported platform !"
#endif

// Detect architecture	
#if defined(__x86_64__)
	#define	APP_ARCH					64						//!< Application architecture
#else
	#define	APP_ARCH					32						//!< Application architecture
#endif	

// Platform specifics
#if defined(APP_WIN)
	#define	FILESEP 					'\\'					//!< File path separator
	#define	FIND_BIN 					"find.exe"				//!< find external utility
	#define	GZIP_BIN 					"gzip.exe"				//!< gzip external utility
	#define	CPIO_BIN 					"cpio.exe"				//!< cpio external utility
	#define	CHMOD_BIN 					"chmod.exe"				//!< cpio external utility
#else	
	#define	FILESEP 					'/'						//!< File path separator
	#define	FIND_BIN 					"find"					//!< find external utility
	#define	GZIP_BIN 					"gzip"					//!< gzip external utility
	#define	CPIO_BIN 					"cpio"					//!< cpio external utility
	#define	DIR_CHMOD					S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH //!< Directory permission when create
#endif

// Usefull common constantes
#define	PATH_MAX_SIZE					256						//!< Maximum size of path
#define	STR_MAX_SIZE					256						//!< Maximum size of common string
#define	CMD_MAX_SIZE					512						//!< Maximum size of command line
#define	BUFFER_SIZE						4096					//!< Maximum size of common buffer

// Commandes constantes
#define CMD_HELP						"help"					//!< Help command
#define CMD_VERSION						"version"				//!< Version command
#define CMD_INFO						"info"					//!< Info command
#define CMD_UNPACK						"unpack"				//!< Unpack command
#define CMD_REPACK						"repack"				//!< Repack command

/**
 * \brief		Command type
 */
typedef enum cmd_type_t{CMD_TYPE_UNKNOWN, CMD_TYPE_BOOT, CMD_TYPE_LOGO}cmd_type_t;

// Default
#define DEFAULT_COMPRESS_RATE			9						//!< Default compression rate
#define DEFAULT_BOOT_IMG_FILENAME		"boot.img"				//!< Default boot input/output filename
#define DEFAULT_LOGO_IMG_FILENAME		"logo.bin"				//!< Default logo input/output filename
#define DEFAULT_KERNEL_FILENAME			"kernel.img"			//!< Default kernel filename
#define DEFAULT_KERNEL_MTK_FILENAME		"kernel.img.mtk"		//!< Default kernel filename with MTK header
#define DEFAULT_RAMDISK_FILENAME		"ramdisk.cpio.gz"		//!< Default ramdisk filename
#define DEFAULT_RAMDISK_MTK_FILENAME	"ramdisk.cpio.gz.mtk"	//!< Default ramdisk filename with MTK header
#define DEFAULT_RAMDISK_DIR				"ramdisk.d"				//!< Default ramdisk directory
#define DEFAULT_LOGOS_DIR				"logos.d"				//!< Default logos directory
#if defined(APP_LINUX)
	#define DEFAULT_CONFIG_FILENAME		"image.cfg"				//!< Default configuration filename
#else
	#define DEFAULT_CONFIG_FILENAME		"image.ini"				//!< Default configuration filename
#endif

// Usefull constantes
#define TMP_RAMDISK_FILENAME			"tmp-ramdisk.cpio.gz"	//!< Temporary ramdisk filename
#define TMP_LOGO_DIR					"tmp-logo.d"			//!< Temporary logo directory
#if defined(APP_LINUX)
	#define LOGO_DB_FILENAME			"mtkimg-logos.cfg"		//!< Logo database filename
#else
	#define LOGO_DB_FILENAME			"mtkimg-logos.ini"		//!< Logo database filename
#endif
#define LOGOS_FILENAME_PREFIX			"img-"					//!< Logos filename prefix


// Boolean type definition
#ifndef bool
	#define bool int											//!< Boolean type definition
	#define true 1												//!< Boolean true
	#define false 0												//!< Boolean false
#endif

/**
 * \brief		Convert boolean to string "yes" or "no"
 * \param		bval	Boolean value to convert
 */	
#define bool2yn(bval) (bval ? "yes" : "no")

/**
 * \brief		Application data structure
 */
typedef struct app_data_t{
	char			exename[PATH_MAX_SIZE];						//!< Executable filename (without path)
	char			working_dir[PATH_MAX_SIZE];					//!< Current working directory
	char			cmd[STR_MAX_SIZE];							//!< Command to execute
	bool			verbose;									//!< Verbose enable flag
	bool			debug;										//!< Debug enable flag
}app_data_t;

/**
 * \brief		Command arguments structure
 */
typedef struct args_t{
	int				argc;										//!< Arguments count
	char**			argv;										//!< Arguments strings array
}args_t;

/**
 * \brief		Image header structure
 */
typedef struct img_header_t{
	char		signature[8];									//!< Image signature (ex: ANDROID!)
	uint32_t 	kernel_size;									//!< Kernel size in bytes
	uint32_t 	kernel_load_addr;								//!< Kernel load address (ex: 0x10008000)
	uint32_t 	ramdisk_size;									//!< Ramdisk size in bytes
	uint32_t 	ramdisk_load_addr;								//!< Ramdisk load address (ex: 0x11000000)
	uint32_t 	second_size;									//!< Second stage size in bytes (Can be 0)
	uint32_t 	second_load_addr;								//!< Second stage load address (0x10F00000)
	uint32_t 	tags_addr;										//!< Tags address (ex: 0x10000100)
	uint32_t 	page_size;										//!< Page size in bytes (ex: 2048)
	uint32_t 	unused1;										//!< Unused (filled with 0x00)
	uint32_t 	unused2;										//!< Unused (filled with 0x00)
	char 		product[16];									//!< Product name (ex: WIKO)
	char 		cmdline[512];									//!< Kernel command line
	uint8_t		id[20];											//!< Image ID to differentiate two ROM
}img_header_t;

/**
 * \brief		MTK header structure
 */
typedef struct mtk_header_t{
	uint8_t		magic[4];										//!< Header magic (0x88 0x16 0x88 0x58)
	uint32_t 	size;											//!< Payload size in bytes
	char 		type[32];										//!< Payload type (ex: KERNEL,ROOTFS,RECOVERY,etc...)
	char 		unused2[472];									//!< Unused, filled with 0xFF
}mtk_header_t;

/**
 * \brief		Image configuration structure
 */
typedef struct img_cfg_t{
	img_header_t	header;										//!< Header informations
	char 			type[32];									//!< Payload type of ramdisk part
	uint32_t 		size;										//!< Size of image
}img_cfg_t;

/**
 * \brief		Logo size database item
 */
typedef struct logo_db_item_t{
	char*			label;										//!< Label of resolution
	unsigned int	width;										//!< Resolution width
	unsigned int	height;										//!< Resolution height
}logo_db_item_t;

/**
 * \brief		Logo size database
 */
typedef struct logo_db_t{
	logo_db_item_t*	items;										//!< Resolutions array
	unsigned int	nitems;										//!< Resolutions count
}logo_db_t	;
	
	
// Include
#include "tools.h"
#include "info.h"
#include "unpack.h"
#include "repack.h"

// Global varisables
app_data_t	app_data;											//!< Global application datas

// Prototypes
int main(int argc, char** argv);
void show_version();
void show_usage();

#endif
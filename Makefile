# *****************************************************************************
#                            MTKIMG PROJECT
# *****************************************************************************
# brief 		Source code builder and management script
# file			Makefile
# author		rom1nux
# date 			2015-07-11
# copyright		Copyright (C) 2015 rom1nux
# *****************************************************************************
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
# *****************************************************************************
# Note for porting :
#  - Add/correct platform detection mechanism after "Detect platform" comment
#  - Add/correct platform specificities after "Platform specifics" comment
#  - Disable 'LOGO_SUPPORT' (LOGO_SUPPORT=0) if you dont have zlib and 
#    libpng1.6 available on your platform.
# *****************************************************************************

# Project 
TITLE=MTKIMG
EXE=mtkimg
AUTHOR=rom1nux

# Project options
LOGO_SUPPORT=1

# Compiler options
CFLAGS=-Wall
LDFLAGS=-lm

# Utilities
SPACE :=
SPACE +=

# Detect platform
ifeq ($(OS),Windows_NT)
	ifneq (,$(findstring /cygdrive/,$(PATH)))
		PLATFORM=CYGWIN
	else
		PLATFORM=WINDOWS
	endif
else
	UNAME=$(shell uname -s)
	ifeq ($(UNAME),Darwin)
		PLATFORM=OSX
	else
		PLATFORM=LINUX
	endif
endif

# Platform specifics
ifeq ($(PLATFORM),WINDOWS)
	# WINDOWS	
	CC=gcc
	RC=windres
	ZIP=7z a -tzip -r
	ZIPEX=-xr!*.git*
	CP=copy
	MV=move
	RM=del /F /Q
	MKDIR=mkdir
	RMDIR=rmdir /S /Q
	CD=cd
	DOX=doxygen
	CAT=type
	EXEC=$(EXE).exe
	FSEP=\\
	TIMESTAMP=$(subst $(SPACE),0,$(shell echo %date:~6,4%%date:~3,2%%date:~0,2%_%time:~0,2%%time:~3,2%%time:~6,2%))
	CWD=$(shell echo %CD%)
	ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
		ARCH=64
	else
		ARCH=32
    endif
	CFLAGS+=-Ilibs/win$(ARCH)/zlib -Ilibs/win$(ARCH)/libpng
	LDFLAGS+=-Llibs/win$(ARCH)/zlib -Llibs/win$(ARCH)/libpng --static
else
	# LINUX/CYGWIN/OSX	
	CC=gcc
	RC=windres
	ZIP=zip -r 
	ZIPEX=--exclude=*.git*
	CP=cp -p
	MV=mv
	RM=rm -f
	MKDIR=mkdir -p
	RMDIR=rm -f -r
	CD=cd
	DOX=doxygen
	CAT=cat
	EXEC=$(EXE)
	FSEP=/
	TIMESTAMP=$(shell date +'%Y%m%d_%H%M%S')
	CWD=$(shell pwd)
endif

# Directories
SRC_DIR=src
BUILD_DIR=build
BKP_DIR=..$(FSEP)bkp
RELEASES_DIR=releases
TMP_DIR=..$(FSEP)tmp

# Files 
BKP_FILE=$(BKP_DIR)$(FSEP)$(EXE)-$(TIMESTAMP).zip

# Options
ifeq ($(LOGO_SUPPORT),1)
	CFLAGS+=-DLOGO_SUPPORT
	LDFLAGS+=-lpng -lz
endif

# Getting sources files
VPATH=$(SRC_DIR)
SRC_FILES=$(wildcard $(SRC_DIR)/*.c)
SRC_OBJS=$(addprefix $(BUILD_DIR)/,$(notdir $(SRC_FILES:.c=.o)))

# Add M$ resources
ifeq ($(PLATFORM),WINDOWS)
	SRC_OBJS+=$(BUILD_DIR)/mtkimg.res
endif
ifeq ($(PLATFORM),CYGWIN)
	SRC_OBJS+=$(BUILD_DIR)/mtkimg.res
endif

# Phony
.PHONY: all clean mrproper rebuild doc release distro backup

# ============================================================================
# TARGET : ALL
# ============================================================================

all: banner $(BUILD_DIR)/$(EXEC)	
	@echo Make '$@' done !

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	@echo Building C file '$@' ...
	$(CC) -c $(CFLAGS) $< -o $@
	
$(BUILD_DIR)/%.res: %.rc Makefile | $(BUILD_DIR)
	@echo Building RC file '$@' ...
	$(RC) $< -O coff -o $@
	
$(BUILD_DIR)/$(EXEC):  $(SRC_OBJS) Makefile | $(BUILD_DIR)
	@echo Linking '$@'...
	$(CC) $(SRC_OBJS) $(LDFLAGS) -o $@
	
$(BUILD_DIR):
	@echo Create directory '$@' ...
	@$(MKDIR) $@	

# ============================================================================
# TARGET : CLEAN
# ============================================================================

clean: banner | $(BUILD_DIR)
	@$(RM) $(BUILD_DIR)$(FSEP)*.o	
	@$(RM) $(BUILD_DIR)$(FSEP)*.res
	@echo Make '$@' done !
	
# ============================================================================
# TARGET : MRPROPER
# ============================================================================

mrproper: banner | $(BUILD_DIR)
	-$(RMDIR) $(BUILD_DIR)	
	@echo Make '$@' done !
	
# ============================================================================
# TARGET : REBUILD
# ============================================================================

rebuild: banner
	$(MAKE) clean
	$(MAKE) all
	$(MAKE) clean
	@echo Make '$@' done !	
	
# ============================================================================
# TARGET : DOC
# ============================================================================

doc: banner
	$(CD) $(SRC_DIR) && \
	$(DOX) doxyfile && \
	$(CD) $(CWD)
	@echo Make '$@' done !		
	
# ============================================================================
# TARGET : RELEASE / SRCRELEASE
# ============================================================================

release: banner rebuild | $(RELEASES_DIR)
	-$(RMDIR) $(TMP_DIR)
	-$(RM) $(RELEASES_DIR)$(FSEP)$(shell $(BUILD_DIR)$(FSEP)$(EXEC) --release).zip
	$(MKDIR) $(TMP_DIR)
	$(CP) $(BUILD_DIR)$(FSEP)$(EXEC) $(TMP_DIR)
	$(CP) readme.txt $(TMP_DIR)
	$(CD) $(TMP_DIR) && \
	$(ZIP) $(CWD)$(FSEP)$(RELEASES_DIR)$(FSEP)$(shell $(BUILD_DIR)$(FSEP)$(EXEC) --release).zip . && \
	$(CD) $(CWD)
	$(RMDIR) $(TMP_DIR)
	@echo Binary release package ready !	

distro: banner rebuild | $(RELEASES_DIR)
	$(shell $(BUILD_DIR)$(FSEP)$(EXEC) --release > $(RELEASES_DIR)$(FSEP)version.txt)
	$(MAKE) mrproper
	$(RM) $(RELEASES_DIR)$(FSEP)$(EXE)-$(word 2,$(subst -, ,$(shell $(CAT) $(RELEASES_DIR)$(FSEP)version.txt)))-src.zip
	$(ZIP) $(RELEASES_DIR)$(FSEP)$(EXE)-$(word 2,$(subst -, ,$(shell $(CAT) $(RELEASES_DIR)$(FSEP)version.txt)))-src.zip . $(ZIPEX)
	$(RM) $(RELEASES_DIR)$(FSEP)version.txt
	@echo Source distro package ready !	
	
$(RELEASES_DIR):
	@echo Create directory '$@' ...
	@$(MKDIR) $@		
	

# ============================================================================
# TARGET : BACKUP
# ============================================================================

backup: banner mrproper | $(BKP_DIR)
	$(ZIP) $(BKP_FILE) .	
	@echo Backup '$(BKP_FILE)' ready !	

$(BKP_DIR):
	@echo Create directory '$@' ...
	@$(MKDIR) $@	
	
# ============================================================================
# TARGET : TEST (For several test purpose during developpement)
# ============================================================================
	
test:	
	@echo $(PLATFORM)	
	
# ============================================================================
# UTILITIES
# ============================================================================	

banner:
	@echo $(TITLE) $(PLATFORM) MAKE by $(AUTHOR) 
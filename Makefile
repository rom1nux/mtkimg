# #############################################################################
#                            MTKIMG PROJECT
# #############################################################################
# file		Makefile
# author	rom1nux
# date 		2015-07-09
# brief 	Source code builder and management script
# #############################################################################

# Project 
TITLE=MTKIMG
EXE=mtkimg
AUTHOR=rom1nux

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
	PLATFORM=LINUX
endif

# Platform specifics
ifeq ($(PLATFORM),WINDOWS)
	# WINDOWS	
	CC=gcc
	RC=windres
	ZIP=7z a -tzip -r
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
else
	# LINUX/CYGWIN	
	CC=gcc
	RC=windres
	ZIP=zip -r
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
DISTRO_DIR=..$(FSEP)distro
TMP_DIR=..$(FSEP)tmp

# Files 
BKP_FILE=$(BKP_DIR)$(FSEP)$(EXE)-$(TIMESTAMP).zip

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
.PHONY: all clean mrproper rebuild doc bindist srcdist backup

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
# TARGET : BINDIST / SRCDIST
# ============================================================================

bindist: banner rebuild | $(DISTRO_DIR)
	-$(RMDIR) $(TMP_DIR)
	-$(RM) $(DISTRO_DIR)$(FSEP)$(shell $(BUILD_DIR)$(FSEP)$(EXEC) --version).zip
	$(MKDIR) $(TMP_DIR)
	$(CP) $(BUILD_DIR)$(FSEP)$(EXEC) $(TMP_DIR)
	$(CP) $(SRC_DIR)$(FSEP)readme.txt $(TMP_DIR)
	$(CD) $(TMP_DIR) && \
	$(ZIP) $(DISTRO_DIR)$(FSEP)$(shell $(BUILD_DIR)$(FSEP)$(EXEC) --version).zip . && \
	$(CD) $(CWD)
	$(RMDIR) $(TMP_DIR)
	@echo Binary distribution ready !	

srcdist: banner rebuild | $(DISTRO_DIR)
	$(shell $(BUILD_DIR)$(FSEP)$(EXEC) --version > $(DISTRO_DIR)$(FSEP)version.txt)
	$(MAKE) mrproper
	$(RM) $(DISTRO_DIR)$(FSEP)$(EXE)-$(word 2,$(subst -, ,$(shell $(CAT) $(DISTRO_DIR)$(FSEP)version.txt)))-src.zip
	$(ZIP) $(DISTRO_DIR)$(FSEP)$(EXE)-$(word 2,$(subst -, ,$(shell $(CAT) $(DISTRO_DIR)$(FSEP)version.txt)))-src.zip .
	$(RM) $(DISTRO_DIR)$(FSEP)version.txt
	@echo Source distribution ready !	
	
$(DISTRO_DIR):
	@echo Create directory '$@' ...
	@$(MKDIR) $@		
	
test:	
	@echo $(word 2,$(subst -, ,mtkimg-0.32-cygwin64))

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
# UTILITIES
# ============================================================================	

banner:
	@echo $(TITLE) $(PLATFORM) MAKE by $(AUTHOR) 
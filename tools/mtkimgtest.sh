#!/bin/bash
# *****************************************************************************
#                            MTKIMG PROJECT
# *****************************************************************************
# brief 	 Automatic quick test suite script
# file		 mtkimgtest.sh
# author	 rom1nux
# date 		 2015-07-09
# copyright	 Copyright (C) 2015 rom1nux
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

# Script constantes
APP_TITLE="MTKIMG TEST SCRIPT"
APP_NAME="mtkimgtest"
APP_VERSION="0.02"
APP_AUTHOR="rom1nux"

# Script configuration
MTKIMG_BIN="mtkimg"				# mtkimg executable
IMAGE_ORG_FILE="org.boot.img" 	# Original image file

# Usefull constantes
COLOR_TITLE="\e[1m\e[34m"
COLOR_ERROR="\e[1m\e[31m"
COLOR_SUCCESS="\e[1m\e[32m"
COLOR_RESET="\e[0m"
TEST_SEPARATOR="--------------------------------------------------------"

# Variables
test_count=0
test_success=0
test_failed=0

# UTILITES
# --------

# Die function
function die()
{
	echo -e "${COLOR_ERROR}ERROR : $@${COLOR_RESET}"
	exit 0;
}

# TESTS UTILITES
# --------------

# Fail function
function test_fail()
{
	test_failed=$((test_failed + 1))
	echo -e "${COLOR_ERROR}* FAIL : $@${COLOR_RESET}"
}

# Fail function
function test_success()
{
	test_success=$((test_success + 1))
	echo -e "${COLOR_SUCCESS}* SUCCESS !${COLOR_RESET}"
}

# Test header
function test_new()
{
	test_count=$((test_count + 1))
	echo $TEST_SEPARATOR
	echo -e "${COLOR_TITLE}* TEST $test_count : $@${COLOR_RESET}"
	test_clean	
}

# Execute mtkimg (normally success)
function test_exec()
{
	$@
	[ $? -eq 0 ] && return 0
	test_fail "Error during '$1' execution !"
	return 1	
}

# Execute mtkimg (normally fail)
function test_exec_fail()
{
	$@
	[ $? -ne 0 ] && return 0
	test_fail "Error during '$1' execution !"
	return 1	
}


# Compare file
function test_compare()
{
	local fa="$1"
	local fb="$2"
	md5a=$(md5sum "$fa"|cut -d' ' -f1)
	md5b=$(md5sum "$fb"|cut -d' ' -f1)
	[ "$md5a" == "$md5b" ] && return 0;
	test_fail "File '$fa' content is different from '$fb' content !"
	return 1
}

# Compare size only
function test_compare_size()
{
	local fa="$1"
	local fb="$2"
	sza=$(stat --printf="%s" "$fa")
	szb=$(stat --printf="%s" "$fb")
	[ "$sza" == "$szb" ] && return 0;
	test_fail "File '$fa' size is différent from '$fb' size !"
	return 1
}

# TESTS FUNCTIONS
# ----------------

function test_clean()
{	
	rm -f "org.kernel.img"
	rm -f "tmp.boot.img"
	rm -f "new.boot.img"
	rm -f "image.cfg"
	rm -f "image.ini"
	rm -f "org.image.cfg"
	rm -f "kernel.img"
	rm -f "org.kernel.img"
	rm -f "org.kernel.img.mtk"	
	rm -f "ramdisk.cpio.gz"
	rm -f "org.ramdisk.cpio.gz"
	rm -f "org.ramdisk.cpio.gz.mtk"
	rm -f -r "ramdisk.d"
	rm -f -r "org.ramdisk.d"
	rm -f "tmp.ramdisk.d.content"
	rm -f "new.ramdisk.d.content"
}

function test_1()
{
	test_new "NO ARGS"
	test_exec_fail ./$MTKIMG_BIN
	[ $? -eq 1 ] && return
	test_success
}

function test_2()
{
	test_new "INFO"
	test_exec ./$MTKIMG_BIN info "$IMAGE_ORG_FILE"
	[ $? -eq 1 ] && return
	test_success
}

function test_3()
{
	test_new "DEFAULT"
	# Unpack
	test_exec ./$MTKIMG_BIN unpack "$IMAGE_ORG_FILE"
	[ $? -eq 1 ] && return
	# Repack
	test_exec ./$MTKIMG_BIN repack "tmp.boot.img"
	[ $? -eq 1 ] && return
	# Compare size only
	test_compare_size "$IMAGE_ORG_FILE" "tmp.boot.img"
	[ $? -eq 1 ] && return
	test_success
}

function test_4()
{
	test_new "NOT EXIST"		
	# Unpack
	test_exec_fail ./$MTKIMG_BIN unpack "iamnotexist.img"
	[ $? -eq 1 ] && return	
	test_success
}

function test_5()
{
	test_new "ALREADY EXIST"		
	# Unpack
	test_exec ./$MTKIMG_BIN unpack "$IMAGE_ORG_FILE" -k "org.kernel.img" -r "org.ramdisk.d" -c "org.image.cfg"
	[ $? -eq 1 ] && return	
	# Repack
	test_exec_fail ./$MTKIMG_BIN repack "$IMAGE_ORG_FILE" -k "org.kernel.img" -r "org.ramdisk.cpio.gz" -c "org.image.cfg"  -n
	[ $? -eq 1 ] && return
	test_success
}

function test_6()
{
	test_new "NO-COMPRESS/NO-DECOMPRESS"		
	# Unpack
	test_exec ./$MTKIMG_BIN unpack "$IMAGE_ORG_FILE" -k "org.kernel.img" -r "org.ramdisk.cpio.gz" -c "org.image.cfg" -n
	[ $? -eq 1 ] && return
	# Repack
	test_exec ./$MTKIMG_BIN repack "tmp.boot.img" -k "org.kernel.img" -r "org.ramdisk.cpio.gz" -c "org.image.cfg"  -n
	[ $? -eq 1 ] && return
	# Compare
	test_compare "$IMAGE_ORG_FILE" "tmp.boot.img"
	[ $? -eq 1 ] && return
	test_success
}

function test_7()
{
	test_new "WITH MTK HEADER"
	# Unpack
	test_exec ./$MTKIMG_BIN unpack "$IMAGE_ORG_FILE" -k "org.kernel.img.mtk" -r "org.ramdisk.cpio.gz.mtk" -c "org.image.cfg" -n -m
	[ $? -eq 1 ] && return
	# Repack
	test_exec ./$MTKIMG_BIN repack "tmp.boot.img" -k "org.kernel.img.mtk" -r "org.ramdisk.cpio.gz.mtk" -c "org.image.cfg"  -n
	[ $? -eq 1 ] && return
	# Compare
	test_compare "$IMAGE_ORG_FILE" "tmp.boot.img"
	[ $? -eq 1 ] && return
	test_success
}

function test_8()
{
	test_new "DECOMPRESS vs MANUAL"
	# Unpack ramdisk file
	test_exec ./$MTKIMG_BIN unpack "$IMAGE_ORG_FILE" -k "org.kernel.img" -r "org.ramdisk.cpio.gz" -c "org.image.cfg" -n
	rm -f "org.kernel.img"
	rm -f "org.image.cfg"
	[ $? -eq 1 ] && return		
	# Unpack ramdisk content
	test_exec ./$MTKIMG_BIN unpack "$IMAGE_ORG_FILE" -k "org.kernel.img" -r "org.ramdisk.d" -c "org.image.cfg"
	[ $? -eq 1 ] && return	
	# Create snapshot
	test_exec tree -apsif "org.ramdisk.d" | sed 's/org.ramdisk.d//' > "tmp.ramdisk.d.content"
	[ $? -eq 1 ] && return	
	# Repack with ramdisk content
	test_exec ./$MTKIMG_BIN repack "tmp.boot.img" -k "org.kernel.img" -r "org.ramdisk.d" -c "org.image.cfg"
	[ $? -eq 1 ] && return
	rm -f -r "org.ramdisk.d"
	
	# Unpack ramdisk content manually
	mkdir "org.ramdisk.d"
	cd "org.ramdisk.d"
	test_exec gzip -d -c ../org.ramdisk.cpio.gz | cpio -i --quiet	
	[ $? -eq 1 ] && return
	cd ..	
	# Create extracted snapshot
	test_exec tree -apsif "org.ramdisk.d" | sed 's/org.ramdisk.d//' > "new.ramdisk.d.content"
	[ $? -eq 1 ] && return
	# Repack with ramdisk content
	test_exec ./$MTKIMG_BIN repack "new.boot.img" -k "org.kernel.img" -r "org.ramdisk.d" -c "org.image.cfg"
	[ $? -eq 1 ] && return	
	
	# Compare extracted snapshot
	test_compare "tmp.ramdisk.d.content" "new.ramdisk.d.content"
	[ $? -eq 1 ] && return	
	# Compare repacked image
	test_compare "tmp.boot.img" "new.boot.img"
	[ $? -eq 1 ] && return
	test_success	
}


# MAIN
# ----

# Banner
echo -e "\n$APP_TITLE V$APP_VERSION - by $APP_AUTHOR"

# Check for original image file
! [ -f "$IMAGE_ORG_FILE" ] && die "Original image '$IMAGE_ORG_FILE' file not found !"

# Do all 
test_1
test_2
test_3
test_4
test_5
test_6
test_7
test_8

# Cleaning
test_clean

# Repport
echo $TEST_SEPARATOR
if [ $test_failed -ne 0 ]; then
	echo -e "${COLOR_ERROR}$test_failed/$test_count tests failed !${COLOR_RESET}"
else
	echo -e "${COLOR_SUCCESS}All $test_count tests successfully passed !${COLOR_RESET}"
fi

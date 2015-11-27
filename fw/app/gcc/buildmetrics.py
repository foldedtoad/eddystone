#!/usr/bin/env python
#
# Reads the build size in relation to the platform allocation size
# Determines whether the code will fit in the allocated flash
# Exits with '0' if it fits, '1' otherwise
#

import os
import sys
import subprocess
import argparse

class ansi_colors:

    '''
    This class provides a convenience wrapper for the color terminal escape
    codes.
    '''

    BLACK    = '\033[90m'
    RED      = '\033[91m'
    GREEN    = '\033[92m'
    YELLOW   = '\033[93m'
    BLUE     = '\033[94m'
    MAGENTA  = '\033[95m'
    CYAN     = '\033[96m'
    WHITE    = '\033[97m'
    ENDC     = '\033[0m'

def colorize(s, color):

    '''
    This routine wraps the given string in terminal escape codes for a color.

    @param s - Supplies the string.
    @param color - Supplies a color escape code.

    @return Returns the escaped string.
    '''

    global useColor

    # If the useColor option is disabled, don't colorize
    if useColor:
        return color + s + ansi_colors.ENDC
    else:
        return s

def read_in_ld_file(filename):
    '''
    This routine reads in the .ld file and parses the size of the various sections

    @param filename - filepath + filename for the .ld file

    @return Returns success if sizes are non-zero, otherwise fails
    '''

    global flashStart
    global flashSize
    global flashEnd
    global ramStart
    global ramSize
    global ramEnd

    # Assumes a well formatted file for example:
    # FLASH  (rx)  : ORIGIN = 0x08030000, LENGTH = 192K
    # RAM    (xrw) : ORIGIN = 0x20000000, LENGTH = 48K

    ldFile = open(filename, "r")

    for line in ldFile:
        if "FLASH" in line:
            for word in line.split():
                if "0x" in word:
                    # Found the start address
                    tempStart = [int(word[2:-1], 16)]
                    flashStart = tempStart[0]
                elif "K" in word:
                    # Found the length
                    tempSize = [int(word[:-1])]
                    flashSize = tempSize[0] * 1024

            #print "FLASH start: 0x{0:08x}".format(flashStart)  # robin
            #print "FLASH size:  {0}".format(flashSize)  # robin

        elif "RAM" in line:
            for word in line.split():
                if "0x" in word:
                    #Found the start address
                    tempStart = [int(word[2:-1], 16)]
                    ramStart = tempStart[0]
                elif "K" in word:
                    #Found the length
                    tempSize = [int(word[:-1])]
                    ramSize = tempSize[0] * 1024

            #print "RAM start: 0x{0:08x}".format(ramStart)  # robin
            #print "RAM size:  {0}".format(ramSize)  # robin

    ldFile.close()

    if flashSize == 0 or ramSize == 0:
        print "ERROR: " + filename + " was not parsed correctly."
        return False
    else:
        flashEnd = flashStart + flashSize
        ramEnd   = ramStart + ramSize

        return True


def get_code_size(filename):

    '''
    This routine reads in the .elf file and parses the size of the various sections

    @param filename - filepath + filename for the .elf file (code binary)

    @return Returns success or failure
    '''

    global flashSize
    global ramSize
    global heapSize
    global bssSize
    global dataSize

    dataStartAddr     = 0
    dataEndAddr       = 0
    bssStartAddr      = 0
    bssEndAddr        = 0
    flashStartAddr    = 0
    flashEndAddr      = 0
    computedFlashSize = 0
    
    # computedRamSize is the sum of bss+data+heap and may differ from 
    # ramSize grabbed from the .ld file by 256 Bytes
    computedRamSize = 0

    toolchain_base = os.environ.get('GNU_INSTALL_ROOT')

    if toolchain_base != None:
        nm_path = toolchain_base + "/bin/arm-none-eabi-nm"
    else:
        nm_path = "arm-none-eabi-nm"

    output = subprocess.Popen([ nm_path, "--print-size", filename], stdout=subprocess.PIPE)

    # Parse the result which is in the following format
    # | Start Address | Size | Type | Name |
    #
    # NOTE: Only the entries with 'size' date are being counted at this point, 
    #       except where explicitly defined

    for line in iter(output.stdout.readline, ''):

        line = line.rstrip()

        # Calculate DATA, BSS, and HEAP specific to RAM
        if "__data_end__" in line:
            for word in line.split():
                # The first block with be the end address ex: 20000428 D __data_end__
                dataEndAddr = int(word, 16)
                break

        if "__data_start__" in line:
            for word in line.split():
                #T he first block with be the start address ex: 20000000 D __data_start__
                dataStartAddr = int(word, 16)
                break

        if "__bss_end__" in line:
            for word in line.split():
                # The first block with be the end address ex: 20003dc8 B __bss_end__
                bssEndAddr = int(word, 16)
                break

        if "__bss_start__" in line:
            for word in line.split():
                # The first block with be the start address ex: 20000428 B __bss_start__
                bssStartAddr = int(word, 16)
                break

        if "_heap_stack_len" in line:
            for word in line.split():
                # The first block with be the size ex: 00008138 A _heap_stack_len
                heapSize = int(word, 16)
                break

        # Calculate FLASH size
        if "__etext" in line:
            for word in line.split():
                # The first block with be the end address ex: 00028c5c R __etext
                flashEndAddr = int(word, 16)
                break

        if "__Vectors" in line:
            for word in line.split():
                # The first block with be the start address ex: 00016000 000000c0 T __Vectors
                flashStartAddr = int(word, 16)
                break

    noErrorFound = True

    dataSize          = dataEndAddr - dataStartAddr
    bssSize           = bssEndAddr - bssStartAddr
    computedRamSize   = bssSize + dataSize + heapSize
    computedFlashSize = flashEndAddr - flashStartAddr

    if computedRamSize > 0:
        dataPercentage = round(float(dataSize)/float(computedRamSize) * 100, 2)
        bssPercentage  = round(float(bssSize)/float(computedRamSize)  * 100, 2)
        heapPercentage = round(float(heapSize)/float(computedRamSize) * 100, 2)
    else:
        print "ERROR: Computed RAM Size is 0."
        return False

    if computedFlashSize <= 0:
        print "ERROR: Computed FLASH size is 0."
        return False

    print "Flash Metrics"
    print "    Flash: {0} bytes used of {1} bytes maximum".format(computedFlashSize, flashSize)

    if flashSize > 0:
        flashPercentage = round(float(computedFlashSize)/float(flashSize) * 100, 2)
    else:
        flashPercentage = 0

    flashPercentageStr = str(flashPercentage)

    # Good Level
    if flashPercentage < 70:
        flashPercentageStr = colorize(flashPercentageStr, ansi_colors.GREEN)

    # Warning Level
    if flashPercentage >= 70 and flashPercentage < 86:
        flashPercentageStr = colorize(flashPercentageStr, ansi_colors.YELLOW)

    # Danger Level
    if flashPercentage >= 86:
        flashPercentageStr = colorize(flashPercentageStr, ansi_colors.RED)

    print "    Flash size used:       {0} %".format(flashPercentageStr)

    print "\nRAM Metrics"
    print "    RAM: {0} bytes used of {1} bytes maximum".format(computedRamSize,ramSize)

    if ramSize > 0:
        ramPercentage = round(float(computedRamSize)/float(ramSize) * 100, 2)
    else:
        ramPercentage = 0

    ramPercentageStr = str(ramPercentage)

    # Good Level
    if ramPercentage < 70:
        ramPercentageStr = colorize(ramPercentageStr, ansi_colors.GREEN)

    # Warning Level
    if ramPercentage >= 70 and ramPercentage < 86:
        ramPercentageStr = colorize(ramPercentageStr, ansi_colors.YELLOW)

    # Danger Level
    if ramPercentage >= 86:
        ramPercentageStr = colorize(ramPercentageStr, ansi_colors.RED)

    print "    RAM used percentage:  {0} %".format(ramPercentageStr)


    return noErrorFound

#=============================================================================================
#Main code path

#Global holders for the allocated starts and sizes for the flash, and ram
flashStart = 0
flashSize = 0
flashEnd = 0
ramStart = 0
ramSize = 0
ramEnd = 0
heapSize = 0
bssSize = 0
dataSize = 0

useColor = True

parser = argparse.ArgumentParser()

parser.add_argument("ldfile", help="Path to linker script")
parser.add_argument("imagefile", help="Path to elf image")
parser.add_argument('-n', '--no-color', action = 'store_true', help = 'Suppress color output.')

args = parser.parse_args()

useColor = not args.no_color

if args.ldfile:
    ldFile = args.ldfile
else:
    sys.exit(1)

if args.imagefile:
    elfFile = args.imagefile
else:
    sys.exit(1)

if(read_in_ld_file(ldFile) == False):
    sys.exit(1)

if(get_code_size(elfFile) == False):
    sys.exit(1)

sys.exit(0)

#------------------------------------------------------------------------------
# Build all components and related items into one unified flash hexfile.
#
# Dependencies:
#    JLink installed and accessed via $PATH
#    intelhex package installed (hexmerge)
#
# hexmerge.py is part of the intelhex python package.
# It is assumed you already have pip installed.
# The command below will install intelhex (and hexmerge.py).
#
#    $ sudo pip install intelhex
#
# NOTE: you may need to do "sudo chmod +x /usr/local/bin/hexmerge.py* "
#
# NOTE: hexmerge can take quite a while to run; up to a minute or longer.
#
# The sequence of operations are --
#    1)   make clean
#    2)   make [debug|release]
#    3)   make flash
# or 
#    1)   make clean
#    2)   make flash
#
# Once the unified hexfile is build, you can repeately flash units with
#         make flash
#
# The products of this makefile are
#    1)  unified.hex
#    2)  unified_<timestamp>.hex
#
# The unitied_<timestamp>.hex is used for distribution.
# The timestamp portion is standard Unix time (GMT) and can be converted via
#      http://www.epochconverter.com
#
#------------------------------------------------------------------------------

SDK_BASE      = ../../..
COMPONENTS    = $(SDK_BASE)/components

SD_DIR     = $(COMPONENTS)/softdevice/s132/hex
SD_NAME    = s132_nrf52_2.0.0-7.alpha_softdevice.hex
SD_HEX     = $(SD_DIR)/$(SD_NAME)
SD_OFF     = 0x00000

APP_DIR    = ../app/gcc
APP_NAME   = application.hex
APP_HEX    = $(APP_DIR)/_build/$(APP_NAME)
APP_OFF    = 0x1C000

BOOT_DIR   = ../bootloader/gcc
BOOT_NAME  = bootloader.hex
BOOT_HEX   = $(BOOT_DIR)/_build/$(BOOT_NAME)
BOOT_OFF   = 0x35000

UNIFIED_DIR    = ./_build
UNIFIED_HEX    = ./_build/unified.hex
UNIFIED_HEX_TS = ./_build/unified_$(BUILD_TYPE)_$(TIMESTAMP).hex

UNIFIED_SD   = $(UNIFIED_DIR)/$(SD_NAME):$(SD_OFF):
UNIFIED_APP  = $(UNIFIED_DIR)/$(APP_NAME):$(APP_OFF):
UNIFIED_BOOT = $(UNIFIED_DIR)/$(BOOT_NAME):$(BOOT_OFF):

#------------------------------------------------------------------------------
#
#------------------------------------------------------------------------------

DEVICE := nRF52832_xxAA
SPEED  := 1000

MAKE := make
COPY := cp
MK   := mkdir -p

JLINK    := JLinkExe -if SWD -speed 1000
HEXMERGE := hexmerge.py

ifeq ($(MAKECMDGOALS),debug)
  BUILD_TYPE = debug
else
  BUILD_TYPE = release
endif

TIMESTAMP := $(shell date +'%s')

# echo suspend
ifeq ("$(VERBOSE)","1")
  NO_ECHO :=
else
  NO_ECHO := @
endif

#------------------------------------------------------------------------------
#
#------------------------------------------------------------------------------
all: $(APP_HEX) $(BOOT_HEX) $(UNIFIED_HEX)

release: all

debug: all

$(APP_HEX):
	$(MAKE) -C $(APP_DIR) -f makefile $(BUILD_TYPE)

$(BOOT_HEX): 
	$(MAKE) -C $(BOOT_DIR)  -f makefile $(BUILD_TYPE)

$(UNIFIED_HEX):
	echo $(UNIFIED_DIR)
	$(MK) $(UNIFIED_DIR)
	$(COPY) $(SD_HEX)    $(UNIFIED_DIR)
	$(COPY) $(APP_HEX)   $(UNIFIED_DIR)
	$(COPY) $(BOOT_HEX)  $(UNIFIED_DIR)
	$(HEXMERGE) $(UNIFIED_SD) $(UNIFIED_APP) $(UNIFIED_BOOT) -o $(UNIFIED_HEX)
	$(COPY) $(UNIFIED_HEX)  $(UNIFIED_HEX_TS)

flash: $(UNIFIED_DIR)/flash.jlink
	$(JLINK) $(realpath $(UNIFIED_DIR)/flash.jlink); test $$? -eq 1

$(UNIFIED_DIR)/flash.jlink: $(APP_HEX) $(BOOT_HEX) $(UNIFIED_HEX)
	@echo "device $(DEVICE)\n"\
		 "speed $(SPEED)\n"\
		 "w4 4001e504 2\n"\
		 "w4 4001e50c 1\n"\
		 "sleep 100\n"\
		 "r\n"\
		 "loadbin \"$(UNIFIED_HEX)\" 0\n"\
		 "r\n"\
		 "g\n"\
		 "exit\n"\
		 > $(UNIFIED_DIR)/flash.jlink

#------------------------------------------------------------------------------
#
#------------------------------------------------------------------------------
clean:
	$(NO_ECHO)$(MAKE) -C $(APP_DIR)  -f makefile clean
	$(NO_ECHO)$(MAKE) -C $(BOOT_DIR) -f makefile clean
	$(NO_ECHO)$(RM) $(UNIFIED_DIR)/*.hex
	$(NO_ECHO)$(RM) $(UNIFIED_DIR)/*.jlink

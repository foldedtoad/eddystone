#------------------------------------------------------------------------------
#  Eddystone for Nordic nRF5x PCA10036 evaluation boards 
#------------------------------------------------------------------------------

TARGET_BOARD         ?= BOARD_PCA10036

PROVISION_DBGLOG     := "yes"

#------------------------------------------------------------------------------
# Define relative paths to SDK components
#------------------------------------------------------------------------------

SDK_BASE      = ../../../..
COMPONENTS    = $(SDK_BASE)/components
TEMPLATE_PATH = $(COMPONENTS)/toolchain/gcc

OUTPUT_NAME   = application
VERSION_NAME  = $(OUTPUT_NAME)_$(BUILD_TYPE)_$(TIMESTAMP)

#------------------------------------------------------------------------------
# Proceed cautiously beyond this point.  Little should change.
#------------------------------------------------------------------------------

export OUTPUT_NAME
export GNU_INSTALL_ROOT

MAKEFILE_NAME := $(MAKEFILE_LIST)
MAKEFILE_DIR := $(dir $(MAKEFILE_NAME) ) 

LINKER_SCRIPT := ./gcc_nrf52_dfu.ld

ifeq ($(OS),Windows_NT)
  include $(TEMPLATE_PATH)/Makefile.windows
else
  include $(TEMPLATE_PATH)/Makefile.posix
endif

ifeq ($(OS),Windows_NT)
	EXT = .exe
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		EXT =
	endif
	ifeq ($(UNAME_S),Darwin)
		EXT = .osx
	endif
endif

# echo suspend
ifeq ("$(VERBOSE)","1")
  NO_ECHO :=
else
  NO_ECHO := @
endif

# Toolchain commands
CC       := "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-gcc"
AS       := "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-as"
AR       := "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-ar" -r
LD       := "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-ld"
NM       := "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-nm"
OBJDUMP  := "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-objdump"
OBJCOPY  := "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-objcopy"
SIZE     := "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-size"
MK       := mkdir
RM       := rm -rf
CP       := cp
GENDAT   := ./gen_dat$(EXT)
GENZIP   := zip

BUILDMETRICS  := ./buildmetrics.py

# function for removing duplicates in a list
remduplicates = $(strip $(if $1,$(firstword $1) $(call remduplicates,$(filter-out $(firstword $1),$1))))

# source common to allADC targets

C_SOURCE_FILES += ../main.c
C_SOURCE_FILES += ../advert.c
C_SOURCE_FILES += ../connect.c
C_SOURCE_FILES += ../eddystone.c
C_SOURCE_FILES += ../battery.c
C_SOURCE_FILES += ../temperature.c
C_SOURCE_FILES += ../printf.c
C_SOURCE_FILES += ../hard_fault_handler.c
C_SOURCE_FILES += ../../bsp/bsp.c
C_SOURCE_FILES += ../dfu_trigger/ble_dfu.c
C_SOURCE_FILES += ../dfu_trigger/bootloader_util_gcc.c
C_SOURCE_FILES += ../dfADCu_trigger/dfu_app_handler.c

ifeq ($(PROVISION_DBGLOG), "yes")
	CFLAGS += -D PROVISION_DBGLOG=1
	C_SOURCE_FILES += ../uart.c
endif

C_SOURCE_FILES += $(COMPONENTS)/libraries/button/app_button.c
C_SOURCE_FILES += $(COMPONENTS)/libraries/util/app_error.c
C_SOURCE_FILES += $(COMPONENTS)/libraries/timer/app_timer.c
C_SOURCE_FILES += $(COMPONENTS)/libraries/timer/app_timer_appsh.c
C_SOURCE_FILES += $(COMPONENTS)/libraries/scheduler/app_scheduler.c
C_SOURCE_FILES += $(COMPONENTS)/libraries/util/nrf_assert.c
C_SOURCE_FILES += $(COMPONENTS)/drivers_nrf/pstorage/pstorage.c
C_SOURCE_FILES += $(COMPONENTS)/drivers_nrf/delay/nrf_delay.c
C_SOURCE_FILES += $(COMPONENTS)/drivers_nrf/common/nrf_drv_common.c
C_SOURCE_FILES += $(COMPONENTS)/drivers_nrf/gpiote/nrf_drv_gpiote.c
C_SOURCE_FILES += $(COMPONENTS)/ble/common/ble_advdata.c
C_SOURCE_FILES += $(COMPONENTS)/ble/common/ble_conn_params.c
C_SOURCE_FILES += $(COMPONENTS)/ble/common/ble_srv_common.c
C_SOURCE_FILES += $(COMPONENTS)/ble/device_manager/device_manager_peripheral.c
C_SOURCE_FILES += $(COMPONENTS)/ble/ble_radio_notification/ble_radio_notification.c
C_SOURCE_FILES += $(COMPONENTS)/toolchain/system_nrf52.c
C_SOURCE_FILES += $(COMPONENTS)/softdevice/common/softdevice_handler/softdevice_handler.c

# assembly files common to all targets
ASM_SOURCE_FILES += $(COMPONENTS)/toolchain/gcc/gcc_startup_nrf52.s

# includes common to all targets
INC_PATHS += -I../
INC_PATHS += -I../dfu_trigger
INC_PATHS += -I../../bsp/
INC_PATHS += -I$(COMPONENTS)/device
INC_PATHS += -I$(COMPONENTS)/toolchain
INC_PATHS += -I$(COMPONENTS)/toolchain/gcc
INC_PATHS += -I$(COMPONENTS)/drivers_nrf/config
INC_PATHS += -I$(COMPONENTS)/drivers_nrf/uart
INC_PATHS += -I$(COMPONENTS)/drivers_nrf/common
INC_PATHS += -I$(COMPONENTS)/drivers_nrf/pstorage
INC_PATHS += -I$(COMPONENTS)/drivers_nrf/pstorage/config
INC_PATHS += -I$(COMPONENTS)/drivers_nrf/delay
INC_PATHS += -I$(COMPONENTS)/drivers_nrf/gpiote
INC_PATHS += -I$(COMPONENTS)/drivers_nrf/saadc
INC_PATHS += -I$(COMPONENTS)/drivers_nrf/hal
INC_PATHS += -I$(COMPONENTS)/drivers_nrf/pstorage
INC_PATHS += -I$(COMPONENTS)/softdevice/s132/headers
INC_PATHS += -I$(COMPONENTS)/softdevice/s132/headers/nrf52
INC_PATHS += -I$(COMPONENTS)/softdevice/common/softdevice_handler
INC_PATHS += -I$(COMPONENTS)/libraries/bootloader_dfu
INC_PATHS += -I$(COMPONENTS)/libraries/util
INC_PATHS += -I$(COMPONENTS)/libraries/uart
INC_PATHS += -I$(COMPONENTS)/libraries/fifo
INC_PATHS += -I$(COMPONENTS)/libraries/trace
INC_PATHS += -I$(COMPONENTS)/libraries/gpiote
INC_PATHS += -I$(COMPONENTS)/libraries/timer
INC_PATHS += -I$(COMPONENTS)/libraries/app_timer_appsh.h
INC_PATHS += -I$(COMPONENTS)/libraries/scheduler
INC_PATHS += -I$(COMPONENTS)/libraries/button
INC_PATHS += -I$(COMPONENTS)/ble/common
INC_PATHS += -I$(COMPONENTS)/ble/ble_services/ble_bas
INC_PATHS += -I$(COMPONENTS)/ble/ble_radio_notification
INC_PATHS += -I$(COMPONENTS)/ble/device_manager
INC_PATHS += -I$(COMPONENTS)/ble/device_manager/config

OBJECT_DIRECTORY = _build
LISTING_DIRECTORY = $(OBJECT_DIRECTORY)
OUTPUT_BINARY_DIRECTORY = $(OBJECT_DIRECTORY)

# Sorting removes duplicates
BUILD_DIRECTORIES := $(sort $(OBJECT_DIRECTORY) $(OUTPUT_BINARY_DIRECTORY) $(LISTING_DIRECTORY) )

ifeq ($(MAKECMDGOALS),debug)
  BUILD_TYPE := debug
endif

ifeq ($(BUILD_TYPE),debug)
  DEBUG_FLAGS += -D DEBUG -g -O0
  CFLAGS += -Wa,-adhln
else
  BUILD_TYPE := release
  DEBUG_FLAGS += -D NDEBUG -O3
  DBGLOG_SUPPORT = "no"
endif

# flags common to all targets
#CFLAGS += -save-temps
CFLAGS += $(DEBUG_FLAGS)
CFLAGS += -D $(TARGET_BOARD)
CFLAGS += -D NRF52
CFLAGS += -D S132
CFLAGS += -D BLE_STACK_SUPPORT_REQD
CFLAGS += -D SOFTDEVICE_PRESENT
CFLAGS += -D BLE_DFU_APP_SUPPORT
CFLAGS += -D CONFIG_GPIO_AS_PINRESET
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs --std=gnu99
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -Wall -Werror
CFLAGS += -Wa,-adhln
CFLAGS += -ffunction-sections 
CFLAGS += -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin

LDFLAGS += -Xlinker -Map=$(LISTING_DIRECTORY)/$(OUTPUT_NAME).map
LDFLAGS += -mthumb -mabi=aapcs -L $(TEMPLATE_PATH) -T$(LINKER_SCRIPT)
LDFLAGS += $(DEBUG_FLAGS)
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += --specs=nano.specs -lc -lnosys
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16

ASMFLAGS += $(DEBUG_FLAGS)
ASMFLAGS += -D $(TARGET_BOARD)
ASMFLAGS += -D NRF52
ASMFLAGS += -D S132
ASMFLAGS += -D SWI_DISABLE0
ASMFLAGS += -D SOFTDEVICE_PRESENT
ASMFLAGS += -D BLE_STACK_SUPPORT_REQD
ASMFLAGS += -D CONFIG_GPIO_AS_PINRESET
ASMFLAGS += -mcpu=cortex-m4
ASMFLAGS += -mthumb -mabi=aapcs --std=gnu99
ASMFLAGS += -x assembler-with-cpp



C_SOURCE_FILE_NAMES = $(notdir $(C_SOURCE_FILES))
C_PATHS = $(call remduplicates, $(dir $(C_SOURCE_FILES) ) )
C_OBJECTS = $(addprefix $(OBJECT_DIRECTORY)/, $(C_SOURCE_FILE_NAMES:.c=.o) )

ASM_SOURCE_FILE_NAMES = $(notdir $(ASM_SOURCE_FILES))
ASM_PATHS = $(call remduplicates, $(dir $(ASM_SOURCE_FILES) ))
ASM_OBJECTS = $(addprefix $(OBJECT_DIRECTORY)/, $(ASM_SOURCE_FILE_NAMES:.s=.o) )

TOOLCHAIN_BASE = $(basename $(notdir $(GNU_INSTALL_ROOT)))

TIMESTAMP = $(shell date +'%s')

vpath %.c $(C_PATHS)
vpath %.s $(ASM_PATHS)

OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

all: $(BUILD_DIRECTORIES) $(OBJECTS)
	@echo Linking target: $(OUTPUT_NAME).elf
	$(NO_ECHO)$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_NAME).elf
	$(NO_ECHO)$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e finalize

	$(NO_ECHO)$(CP) $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_NAME).hex $(OUTPUT_BINARY_DIRECTORY)/$(VERSION_NAME).hex
	$(NO_ECHO)$(CP) $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_NAME).bin $(OUTPUT_BINARY_DIRECTORY)/$(VERSION_NAME).bin
	$(NO_ECHO)$(CP) $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_NAME).dat $(OUTPUT_BINARY_DIRECTORY)/$(VERSION_NAME).dat
	$(NO_ECHO)$(CP) $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_NAME).zip $(OUTPUT_BINARY_DIRECTORY)/$(VERSION_NAME).zip

	@echo "*****************************************************"
	@echo "build project: $(OUTPUT_NAME)"
	@echo "build type:    $(BUILD_TYPE)"
	@echo "build with:    $(TOOLCHAIN_BASE)"
	@echo "build target:  $(TARGET_BOARD)"
	@echo "build SOC:     $(TARGET_SOC)"
	@echo "build options  --"
	@echo "               PROVISION_DBGLOG   $(PROVISION_DBGLOG)"
	@echo "build products --"
	@echo "               $(OUTPUT_NAME).elf"
	@echo "               $(OUTPUT_NAME).hex"
	@echo "               $(OUTPUT_NAME).bin"
	@echo "               $(OUTPUT_NAME).dat"
	@echo "               $(OUTPUT_NAME).zip"
	@echo "build versioning --"
	@echo "               $(VERSION_NAME).hex"
	@echo "               $(VERSION_NAME).bin"
	@echo "               $(VERSION_NAME).dat"
	@echo "               $(VERSION_NAME).zip"
	@echo "*****************************************************"

debug : all

release : all

# Create build directories
$(BUILD_DIRECTORIES):
	echo $(MAKEFILE_NAME)
	$(MK) $@

# Create objects from C SRC files
$(OBJECT_DIRECTORY)/%.o: %.c
	@echo Compiling file: $(notdir $<)
	$(NO_ECHO)$(CC) $(CFLAGS) $(INC_PATHS) \
	-c $< -o $@ > $(OUTPUT_BINARY_DIRECTORY)/$*.lst

# Assemble files
$(OBJECT_DIRECTORY)/%.o: %.s
	@echo Compiling file: $(notdir $<)
	$(NO_ECHO)$(CC) $(ASMFLAGS) $(INC_PATHS) -c -o $@ $<

# Link
$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_NAME).elf: $(BUILD_DIRECTORIES) $(OBJECTS)
	@echo Linking target: $(OUTPUT_NAME).elf
	@echo LINKER_SCRIPT: $(LINKER_SCRIPT)
	$(NO_ECHO)$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_NAME).elf

# Create binary .bin file from the .elf file
$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_NAME).bin: $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_NAME).elf
	@echo Preparing: $(OUTPUT_NAME).bin
	$(NO_ECHO)$(OBJCOPY) -O binary $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_NAME).elf $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_NAME).bin

# Create binary .hex file from the .elf file
$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_NAME).hex: $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_NAME).elf
	@echo Preparing: $(OUTPUT_NAME).hex
	$(NO_ECHO)$(OBJCOPY) -O ihex $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_NAME).elf $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_NAME).hex

finalize: genbin genhex gendat genzip echosize memory_report

# Analyze elf file's idea of memory usage
memory_report: $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_NAME).elf
	-@echo ""
	-@$(BUILDMETRICS) \
		$(LINKER_SCRIPT) \
		$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_NAME).elf
	-@echo ""

genbin:
	@echo Preparing: $(OUTPUT_NAME).bin
	$(NO_ECHO)$(OBJCOPY) -O binary $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_NAME).elf $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_NAME).bin

# Create binary .hex file from the .elf file
genhex: 
	@echo Preparing: $(OUTPUT_NAME).hex
	$(NO_ECHO)$(OBJCOPY) -O ihex $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_NAME).elf $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_NAME).hex

# Create .dat file from the .bin file
gendat: 
	@echo Preparing: $(OUTPUT_NAME).dat
	$(NO_ECHO)$(GENDAT) $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_NAME).bin $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_NAME).dat

# Create .zip file from the .bin + .dat files
genzip: 
	@echo Preparing: $(OUTPUT_NAME).zip
	-@$(GENZIP) -j $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_NAME).zip $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_NAME).bin $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_NAME).dat

echosize:
	-@echo ""
	$(NO_ECHO)$(SIZE) $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_NAME).elf
	-@echo ""

clean:
	$(RM) $(BUILD_DIRECTORIES)

cleanobj:
	$(RM) $(BUILD_DIRECTORIES)/*.o


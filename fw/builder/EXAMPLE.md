# Builder Example

###"Builder" build step    
Below is an example of the output from the "builder" make process.  
This builds a "unified" firmware hexfile, which consolidates the softdevice, appplication, and bootloader components into a single hexfile image.  

```
  $ make clean  
  $ make debug  
```  
  
The final product of the "builder" is a versioned hexfile: unified_debug_1449000685.hex in example below. Note that the hexfile has a standard unix-type timestamp, in this example "1449000685", which encodes the build time.

This unified hexfile can then be flashed onto the device in a single operation. This is often needed for high-volume, production manufacturability.  It is also useful for just getting all the firmware parts initially onto the PCA10028 device.  

```
knots:nrf51_sdk_v8 robin$ cd eddystone/fw/builder

knots:builder robin$ make clean
make -C ../app/gcc  -f makefile clean
rm -rf _build
make -C ../bootloader/gcc -f makefile clean
rm -rf _build
rm -f ./_build/*.hex
rm -f ./_build/*.jlink

knots:builder robin$ make debug
make -C ../app/gcc -f makefile debug
echo  makefile
makefile
mkdir _build
Compiling file: main.c
Compiling file: advert.c
Compiling file: connect.c
Compiling file: eddystone.c
Compiling file: battery.c
Compiling file: temperature.c
Compiling file: printf.c
Compiling file: hard_fault_handler.c
Compiling file: bsp.c
Compiling file: ble_dfu.c
Compiling file: bootloader_util_gcc.c
Compiling file: dfu_app_handler.c
Compiling file: uart.c
Compiling file: app_button.c
Compiling file: app_error.c
Compiling file: app_fifo.c
Compiling file: app_timer.c
Compiling file: app_timer_appsh.c
Compiling file: app_scheduler.c
Compiling file: pstorage.c
Compiling file: nrf_assert.c
Compiling file: app_uart_fifo.c
Compiling file: nrf_delay.c
Compiling file: nrf_drv_common.c
Compiling file: ble_advdata.c
Compiling file: ble_conn_params.c
Compiling file: ble_srv_common.c
Compiling file: ble_radio_notification.c
Compiling file: system_nrf51.c
Compiling file: softdevice_handler.c
Compiling file: device_manager_peripheral.c
Compiling file: app_gpiote.c
Compiling file: gcc_startup_nrf51.s
Linking target: application.elf
Preparing: application.bin
Preparing: application.hex
Preparing: application.dat
Preparing: application.zip
  adding: application.bin (deflated 52%)
  adding: application.dat (deflated 13%)

   text	   data	    bss	    dec	    hex	filename
  38504	    132	   2940	  41576	   a268	_build/application.elf


Flash Metrics
    Flash: 38504 bytes used of 118784 bytes maximum
    Flash size used:       32.42 %

RAM Metrics
    RAM: 3072 bytes used of 24576 bytes maximum
    RAM used percentage:  12.5 %

*****************************************************
build project: application
build type:    debug
build with:    gcc-arm-none-eabi-4_9-2014q4
build target:  BOARD_PCA10028
build SOC:     xxac
build options  --
               PROVISION_DBGLOG   yes
build products --
               application.elf
               application.hex
               application.bin
               application.dat
               application.zip
build versioning --
               application_debug_1449000689.hex
               application_debug_1449000689.bin
               application_debug_1449000689.dat
               application_debug_1449000689.zip
*****************************************************
make -C ../bootloader/gcc  -f makefile debug
echo  makefile
makefile
mkdir _build
Compiling file: main.c
Compiling file: dfu_init.c
Compiling file: dfu_ble_svc.c
Compiling file: dfu_transport_ble.c
Compiling file: bootloader_util_gcc.c
Compiling file: bootloader.c
Compiling file: dfu_single_bank.c
Compiling file: ble_radio_notification.c
Compiling file: ble_conn_params.c
Compiling file: ble_advdata.c
Compiling file: ble_srv_common.c
Compiling file: ble_dis.c
Compiling file: ble_bas.c
Compiling file: ble_dfu.c
Compiling file: nrf_delay.c
Compiling file: pstorage.c
Compiling file: crc16.c
Compiling file: hci_mem_pool.c
Compiling file: app_scheduler.c
Compiling file: app_gpiote.c
Compiling file: app_timer.c
Compiling file: app_timer_appsh.c
Compiling file: softdevice_handler.c
Compiling file: softdevice_handler_appsh.c
Compiling file: system_nrf51.c
Compiling file: printf.c
Compiling file: uart.c
Compiling file: gcc_startup_nrf51.s
Linking target: bootloader.elf
Preparing: bootloader.bin
Preparing: bootloader.hex

   text	   data	    bss	    dec	    hex	filename
  37352	   1132	   4672	  43156	   a894	_build/bootloader.elf

Linking target: bootloader.elf

Flash Metrics
    Flash: 37352 bytes used of 45056 bytes maximum
    Flash size used:       82.9 %

RAM Metrics
    RAM: 4776 bytes used of 8192 bytes maximum
    RAM used percentage:  58.3 %

*****************************************************
build project:  bootloader
build type:     debug
build with:     gcc-arm-none-eabi-4_9-2014q4
build target:   BOARD_PCA10028
build options   --
                DBGLOG_SUPPORT    yes
build products: --
                bootloader.elf
                bootloader.hex
build versioning --
               bootloader_debug_1449000690.hex
*****************************************************
echo ./_build
./_build
mkdir -p ./_build
cp ./s110_nrf51_8.0.0_softdevice.hex    ./_build
cp ../app/gcc/_build/application.hex   ./_build
cp ../bootloader/gcc/_build/bootloader.hex  ./_build
hexmerge.py ./_build/s110_nrf51_8.0.0_softdevice.hex:0x00000: ./_build/application.hex:0x18000: ./_build/bootloader.hex:0x35000: -o ./_build/unified.hex
cp ./_build/unified.hex  ./_build/unified_debug_1449000685.hex
knots:builder robin$ 
knots:builder robin$ 
```
###"Builder" flash step  
After you use the "builder" to create the unified hexfile image, you then flash it onto the PCA10028 device.
From the same directory, just enter

```
  $ make flash
```  
  
And, if your JLink utilities are install properly, the makefile script will flash the unified hexfile onto the device.  
Below is an example of this procedure.  
  
```
knots:builder robin$ make flash
JLinkExe -if SWD /Users/robin/nordic/nrf51_sdk_v8/eddystone/fw/builder/_build/flash.jlink; test $? -eq 1
SEGGER J-Link Commander V4.98e ('?' for help)
Compiled May  5 2015 11:59:45

Script file read successfully.
DLL version V4.98e, compiled May  5 2015 11:59:38
Firmware: J-Link OB-SAM3U128-V2-NordicSemi compiled Aug 28 2015 19:26:24
Hardware: V1.00
S/N: xxxxxxxxx 
VTarget = 3.300V
Info: Found SWD-DP with ID 0x0BB11477
Info: Found Cortex-M0 r0p0, Little endian.
Info: FPUnit: 4 code (BP) slots and 0 literal slots
Info: CoreSight components:
Info: ROMTbl 0 @ F0000000
Info: ROMTbl 0 [0]: F00FF000, CID: B105100D, PID: 000BB471 ROM Table
Info: ROMTbl 1 @ E00FF000
Info: ROMTbl 1 [0]: FFF0F000, CID: B105E00D, PID: 000BB008 SCS
Info: ROMTbl 1 [1]: FFF02000, CID: B105E00D, PID: 000BB00A DWT
Info: ROMTbl 1 [2]: FFF03000, CID: B105E00D, PID: 000BB00B FPB
Info: ROMTbl 0 [1]: 00002000, CID: B105900D, PID: 000BB9A3 ???
Cortex-M0 identified.
Target interface speed: 100 kHz
Processing script file...

Info: Device "NRF51822_XXAA" selected.
Reconnecting to target...
Info: Found SWD-DP with ID 0x0BB11477
Info: Found Cortex-M0 r0p0, Little endian.
Info: FPUnit: 4 code (BP) slots and 0 literal slots
Info: CoreSight components:
Info: ROMTbl 0 @ F0000000
Info: ROMTbl 0 [0]: F00FF000, CID: B105100D, PID: 000BB471 ROM Table
Info: ROMTbl 1 @ E00FF000
Info: ROMTbl 1 [0]: FFF0F000, CID: B105E00D, PID: 000BB008 SCS
Info: ROMTbl 1 [1]: FFF02000, CID: B105E00D, PID: 000BB00A DWT
Info: ROMTbl 1 [2]: FFF03000, CID: B105E00D, PID: 000BB00B FPB
Info: ROMTbl 0 [1]: 00002000, CID: B105900D, PID: 000BB9A3 ???

Target interface speed: 1000 kHz

Writing 00000002 -> 4001E504

Writing 00000001 -> 4001E50C

Sleep(100)

Reset delay: 0 ms
Reset type NORMAL: Resets core & peripherals via SYSRESETREQ & VECTRESET bit.

Downloading file [./_build/unified.hex]...Info: J-Link: Flash download: Flash programming performed for 6 ranges (169984 bytes)
Info: J-Link: Flash download: Total time needed: 2.900s (Prepare: 0.096s, Compare: 0.074s, Erase: 0.000s, Program: 2.697s, Verify: 0.022s, Restore: 0.009s)
O.K.

Reset delay: 0 ms
Reset type NORMAL: Resets core & peripherals via SYSRESETREQ & VECTRESET bit.



Script processing completed.

make: *** [flash] Error 1
knots:builder robin$ 
```

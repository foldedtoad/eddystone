# Builder Example

###"Builder" build step    
Below is an example of the output from the "builder" make process.  
This builds a "unified" firmware hexfile, which consolidates the softdevice, appplication, and bootloader components into a single hexfile image.  

```
  $ make clean  
  $ make debug  
```  
  
The final product of the "builder" is a versioned hexfile: unified_debug_1449000685.hex in example below. Note that the hexfile has a standard unix-type timestamp, in this example "1449000685", which encodes the build time.

This unified hexfile can then be flashed onto the device in a single operation. This is often needed for high-volume, production manufacturability.  It is also useful for just getting all the firmware parts initially onto the PCA10036 device.  

```
robin@mandible:~/nrf52/sdk9/eddystone/fw/builder$ make clean
rm -rf _build
rm -rf _build

robin@mandible:~/nrf52/sdk9/eddystone/fw/builder$ make debug
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
Compiling file: app_timer.c
Compiling file: app_timer_appsh.c
Compiling file: app_scheduler.c
Compiling file: nrf_assert.c
Compiling file: pstorage.c
Compiling file: nrf_delay.c
Compiling file: nrf_drv_common.c
Compiling file: nrf_drv_gpiote.c
Compiling file: ble_advdata.c
Compiling file: ble_conn_params.c
Compiling file: ble_srv_common.c
Compiling file: device_manager_peripheral.c
Compiling file: ble_radio_notification.c
Compiling file: system_nrf52.c
Compiling file: softdevice_handler.c
Compiling file: gcc_startup_nrf52.s
Linking target: application.elf
Preparing: application.bin
Preparing: application.hex
Preparing: application.dat
Preparing: application.zip
  adding: application.bin (deflated 53%)
  adding: application.dat (deflated 19%)

   text    data     bss     dec     hex filename
  38392     132    2616   41140    a0b4 _build/application.elf

Linking target: application.elf
LINKER_SCRIPT: ./gcc_nrf52_dfu.ld

Flash Metrics
    Flash: 38392 bytes used of 102400 bytes maximum
    Flash size used:       37.49 %

RAM Metrics
    RAM: 2748 bytes used of 24576 bytes maximum
    RAM used percentage:  11.18 %

*****************************************************
build project: application
build type:    debug
build with:    gcc-arm-none-eabi-5_2-2015q4
build target:  BOARD_PCA10036
build SOC:     
build options  --
               PROVISION_DBGLOG   yes
build products --
               application.elf
               application.hex
               application.bin
               application.dat
               application.zip
build versioning --
               application_debug_1456985970.hex
               application_debug_1456985970.bin
               application_debug_1456985970.dat
               application_debug_1456985970.zip
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
Compiling file: system_nrf52.c
Compiling file: printf.c
Compiling file: uart.c
Compiling file: gcc_startup_nrf52.s
Linking target: bootloader.elf
Preparing: bootloader.bin
Preparing: bootloader.hex

   text    data     bss     dec     hex filename
  38028    1136    4640   43804    ab1c _build/bootloader.elf

Linking target: bootloader.elf

Flash Metrics
    Flash: 38028 bytes used of 45056 bytes maximum
    Flash size used:       84.4 %

RAM Metrics
    RAM: 4748 bytes used of 24576 bytes maximum
    RAM used percentage:  19.32 %

*****************************************************
build project:  bootloader
build type:     debug
build with:     gcc-arm-none-eabi-5_2-2015q4
build target:   BOARD_PCA10036
build options   --
                DBGLOG_SUPPORT    yes
build products: --
                bootloader.elf
                bootloader.hex
build versioning --
               bootloader_debug_1456985971.hex
*****************************************************
echo ./_build
./_build
mkdir -p ./_build
cp ./s132_nrf52_2.0.0_softdevice.hex    ./_build
cp ../app/gcc/_build/application.hex   ./_build
cp ../bootloader/gcc/_build/bootloader.hex  ./_build
hexmerge.py ./_build/s132_nrf52_2.0.0_softdevice.hex:0x00000: ./_build/application.hex:0x1C000: ./_build/bootloader.hex:0x35000: -o ./_build/unified.hex
cp ./_build/unified.hex  ./_build/unified_debug_1456985967.hex
 
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
S/N: xxxxxxxx 
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

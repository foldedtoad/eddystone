# eddystone
Eddystone on Nordic PCA10028 device

##Overview of Operation  
This Eddystone implementation operated in two modes.  Upon power-on or reset, the device will advertise in "connectable" phase.  After an advertisement timeout (currently 30 seconds), the device is reconfigured to run in standard eddystone "non-connectable" phase for remainder of the power-on cycle.

During the initial "connectable" phase, the device offers only the Nordic "DFU" service.  So, for example, you can connect to the device with the Android app "Nordic Master Control Panel (MCP)" and start a DFU (Device Firmware Update). This triggers the device to return to the Bootloader where the actual DFU processing will take place.

Once the Bootloader is in control, it will advertise itself as "DfuTarg".  You may need rescan with the MCP app to see this new advertised name.

##Building the Firmware  

###Requirements  
* Recent version of GCC toolchain.
* Python 2.7 (2.6 may be ok).
* intelhex which includes hexmerge: "sudo pip install intelhex".
* Segger JLink utilities -- should be part of Nordic's nRFgo-Studio install.

###Project Placement  
This project is arranged to expect to be placed (cloned) into the Nordic nRF51 SDK folder on a peer level with the "component", "example", etc. subfolders.

```
  .
  ├── components
  ├── eddystone
  │   ├── README.md
  │   ├── android
  │   │   └── EddystoneValidator
  │   ├── fw
  │   │   ├── app
  │   │   ├── bootloader
  │   │   └── builder
  │   └── ios
  │       └── ios-eddystone-scanner-sample
  └── examples
```


**Builder Example**  
Below is an example of the output from the "builder" make process.  
This builds a "unified" firmware hex file, which consolidates the softdevice, appplication, and bootloader components into a single hex file image. This one unified hexfile can them be flashed onto the device in a single operation. This is often needed for high-volume, production manufacturability.  It is also usefull for just getting all the firmware parts initially onto the PCA10028 device.
  
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
  
  

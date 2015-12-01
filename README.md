# Eddystone
Eddystone on Nordic PCA10028 device

##Overview of Operation  
This Eddystone implementation operated in two modes.  Upon power-on or reset, the device will advertise in "connectable" phase.  After an advertisement timeout (currently 30 seconds), the device is reconfigured to run in standard eddystone "non-connectable" phase for remainder of the power-on cycle.

During the initial "connectable" phase, the device offers only the Nordic "DFU" service.  So, for example, you can connect to the device with the Android app "Nordic Master Control Panel (MCP)" and start a DFU (Device Firmware Update). This triggers the device to return to the Bootloader where the actual DFU processing will take place.

Once the Bootloader is in control, it will advertise itself as "DfuTarg".  You may need rescan with the MCP app to see this new advertised name.

##Building the Firmware  

###Requirements  
It is assumed that you are able to build the Nordic SDK examples at this point.  
Additional requirement are --

* Recent version of GCC toolchain.
* Python 2.7 (2.6 may be ok).
* intelhex which includes hexmerge: "sudo pip install intelhex".
* Segger JLink utilities: version V4.98e or later (slighly earlier versions may work too).

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

##Building Eddystone  

###Building with "Builder"
The easiest way to start is by building and installing the firmware via the "builder" procedure.
Please read the README.md and EXAMPLE.md files under the ./fw/builder directory.

###Developer builds
Enter the component directory you wish to build: app or firmware.
Within this directory there is a subdirectory "gcc"; enter it.
This gcc directory will be where you run the makefile.
The makefile supports two build types: debug and release, which release being the default. Examples are given below.

```
  $ make clean  
  $ make debug  
```
or  
```
  $ make clean  
  $ make   
```  

##OTA-DFU support
This project incorporates OTA-DFU support (Over-The-Air Device-Firmware-Update).
See the HOWTO_DFU.md file under the ./fw/app directory for details.





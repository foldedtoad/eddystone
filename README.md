# eddystone
Eddystone on Nordic PCA10028 device

**Overview of Operation**  
This Eddystone implementation operated in two modes.  Upon power-on or reset, the device will advertise in "connectable" phase.  After an advertisement timeout (currently 30 seconds), the device is reconfigured to run in standard eddystone "non-connectable" phase for remainder of the power-on cycle.

During the initial "connectable" phase, the device offers only the Nordic "DFU" service.  So, for example, you can connect to the device with the Android app "Nordic Master Control Panel (MCP)" and start a DFU (Device Firmware Update). This triggers the device to return to the Bootloader where the actual DFU processing will take place.

Once the Bootloader is in control, it will advertise itself as "DfuTarg".  You may need rescan with the MCP app to see this new advertised name.

**Building the Firmware**
This project is arranged to expect to be placed (cloned) in the Nordic nRF51 SKD folder on a peer level with the "component", "example", etc. subfolders.

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
  

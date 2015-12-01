# How-to OTA-DFU
This outlines the procedures to do an Over-The-Air Device-Fireware-Update (OTA-DFU).

###Assumptions
It is assumed that you have --

* An Android device which support Bluetooth LE.
* Installed the Nordic Master Control Panel app (MCP) on your Android device.
* Built the "app" firmware component and have an "application_xxxxxx.zip" file.
* (optional, recommended) Have access to Google Drive, where you have copies your "application_xxxxx.zip" file.

###Transistioning to the Bootloader

1. Start the “Master Control Panel” app.  
 1.1. From this “Devices List” page, select the PCA10028 device of interest: “CONNECT”  

2. Connect to device with Android “Master Control Panel” app.  

3. Expand the “Generic Attribute” service  
 3.1. Select the “Service Changed” indicate. Down-Up-arrows icon should have X.  

4. Expand the “Device Firmware Update Service” service.  
 4.1. On the “DFU Control Point”, select the 3-down-arrows icon.  
 4.2. On the “DFU Control Point”, select the up-arrow icon.  
 4.2.1. From the pop-up, select the “Application” checkbox and then OK.  

###Installing the Firmware

1. Return (back-arrow) to the Master Control Panel” Device List page and tap “scan”  
1.1. You should see the “DfuTarg” device listed; select it, e.g. connect to this device.  
1.2. Note: sometime you may need to tap the “connect” a second time to establish a connection: appears to be a MCP app bug.  
2. In the upper right corner of this page, you will see the “DFU” icon; tap this icon.  
2.1. From the “Select file type”, tap the “Multiple files (ZIP) radio button.  

3. You are now going to select the firmware file from your Google Drive.  
3.1. Select the “Google Drive” icon  
3.1.1. Select the “application_release_<timestamp>.zip file  

4. You should now see a new page which has a graph on it.  
4.1. You should see  
4.1.1. Initializing…  
4.1.2. Connecting…  
4.1.3. Starting DFU…  
4.1.4. xx%  

5. After it completes (100%)  


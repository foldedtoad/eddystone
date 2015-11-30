###How to flash a device via Windows command prompt  

*Made for a Windows 7 PC with proper JLink files installed.  

Files required:  

      unified hex file  
      flasher.cmd file  

Download SEGGER files for JLink adapter: https://www.segger.com/jlink-software.html?step=1&file=JLink_492  

Go to **Start**-->RIGHT click **Computer**-->**Properties**  
In sidebar, click **Advanced system settings** and then **Environment Variables.**  
Under **System Variables**, select **Path variable** and then **Edit...**  
At the end of the Path line, add   

      ;C:\Program Files (x86)\SEGGER\JLink_V492\  

This path can be found by clicking **Start**-->**All Programs**-->**SEGGER**-->**J-Link V4.92**-->RIGHT click **J-Link Commander**-->**Properties**.  
The **Target** field contains the path.   
Make sure the quotes as well as "JLink.exe" are removed if copying directly. Click OK.  

Open new command prompt shell.  

Navigate to folder containing flasher.cmd and .hex files. Run flasher like so:  

      > flasher.cmd unified.hex  

If the files are not in the same directory, explicitly specify the path to the .hex file:  

      e.g. > flasher.cmd _build/unified.hex  

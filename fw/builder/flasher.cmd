REM
REM  Standalone flash utility for unified images. (PC version)
REM
REM  Ex.  ./flasher <unified_hexfile.hex path>
REM
echo device nRF51822_xxAA> %TEMP%/unified_flash.jlink
echo speed 1000     >> %TEMP%/unified_flash.jlink
echo w4 4001e504 2  >> %TEMP%/unified_flash.jlink
echo w4 4001e50c 1  >> %TEMP%/unified_flash.jlink
echo sleep 100      >> %TEMP%/unified_flash.jlink
echo r              >> %TEMP%/unified_flash.jlink
echo loadbin %1 0   >> %TEMP%/unified_flash.jlink
echo r              >> %TEMP%/unified_flash.jlink
echo g              >> %TEMP%/unified_flash.jlink
echo exit           >> %TEMP%/unified_flash.jlink

JLink.Exe -if SWD %TEMP%/unified_flash.jlink


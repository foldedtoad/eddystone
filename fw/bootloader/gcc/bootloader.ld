/* Linker script to configure memory regions. */
SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

/*
 *   SoftDevice         0x00000 - 0x17FFF    length 96K
 *   App                0x18000 - 0x34FFF    length 116K
 *   Bootloader         0x35000 - 0x3FFFF    length 44k
 */

MEMORY
{
    FLASH (rx) : ORIGIN = 0x00035000, LENGTH = 44K
    RAM (rwx)  : ORIGIN = 0x20002000, LENGTH = 24K

    BOOTLOADER_SETTINGS (rw)  : ORIGIN = 0x0003FC00, LENGTH = 1K
    NRF_UICR_BOOT_START (rwx) : ORIGIN = 0x10001014, LENGTH = 4
}

SECTIONS
{
    .bootloaderSettings 0x0003FC00 :
    {

    }
    .uicrBootStartAddress 0x10001014 :
    {

    } 
}

INCLUDE "nrf5x_common.ld"
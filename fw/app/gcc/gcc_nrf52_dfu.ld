/* Linker script to configure memory regions. */
/* This script allocates the top region for the bootloader image */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys -lm)

/*
 *   SoftDevice         0x00000 - 0x17FFF    length 96K
 *   App                0x1C000 - 0x34FFF    length 100K
 *   Bootloader         0x35000 - 0x3FFFF    length 44k
 */

MEMORY
{
  FLASH (rx) : ORIGIN = 0x0001C000, LENGTH = 100K 
  RAM (rwx) :  ORIGIN = 0x20002000, LENGTH = 24K 
}

INCLUDE "gcc_nrf51_common.ld"

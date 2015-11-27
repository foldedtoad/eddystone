/* Linker script to configure memory regions. */
/* This script allocates the top region for the bootloader image */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys -lm)

/*
 *   SoftDevice         0x00000 - 0x17FFF    length 96K
 *   App                0x18000 - 0x34FFF    length 116K
 *   Bootloader         0x35000 - 0x3FFFF    length 44k
 */

MEMORY
{
  FLASH (rx) : ORIGIN = 0x00018000, LENGTH = 116K 
  RAM (rwx) :  ORIGIN = 0x20002000, LENGTH = 24K 
}

INCLUDE "gcc_nrf51_common.ld"

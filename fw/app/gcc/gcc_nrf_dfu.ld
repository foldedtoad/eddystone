/* Linker script to configure memory regions. */
/* This script allocates the top region for the bootloader image */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys -lm)

/*
 *   nRF52832:  512K Flash,  64K RAM
 *
 *   SoftDevice             0x00000 - 0x1AFFF    length 111K
 *   App                    0x1B000 - 0x71FFF    length 345K
 *   Bootloader             0x72000 - 0x7EFFF    length 52K
 *   bootloadder_settings   0x7E000 - 0x7FFFF    length 4K
 */

MEMORY
{
  FLASH (rx) : ORIGIN = 0x0001B000, LENGTH = 100K 
  RAM (rwx) :  ORIGIN = 0x20002000, LENGTH = 24K 
}

SECTIONS
{
  .fs_data_out ALIGN(4):
  {
    PROVIDE( __start_fs_data = .);
    KEEP(*(fs_data))
    PROVIDE( __stop_fs_data = .);
  } = 0
}

INCLUDE "nrf5x_common.ld"

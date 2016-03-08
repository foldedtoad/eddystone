/* Linker script to configure memory regions. */
SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

/*
 *   nRF52832:  512K Flash,  64K RAM
 *
 *   SoftDevice             0x00000 - 0x1BFFF    length 112K
 *   App                    0x1C000 - 0x71FFF    length 344K
 *   Bootloader             0x72000 - 0x7EFFF    length 52K
 *   bootloadder_settings   0x7E000 - 0x7FFFF    length 4K
 */

MEMORY
{
  FLASH               (rx)  : ORIGIN = 0x72000,    LENGTH = 52K
  RAM                 (rwx) : ORIGIN = 0x20002C00, LENGTH = 0x5380
  NOINIT              (rwx) : ORIGIN = 0x20007F80, LENGTH = 0x80
  BOOTLOADER_SETTINGS (rw)  : ORIGIN = 0x0007F000, LENGTH = 4K
  UICR_BOOTLOADER     (r)   : ORIGIN = 0x10001014, LENGTH = 4
  MBR_PARAMS_PAGE     (rw)  : ORIGIN = 0x0007E000, LENGTH = 4K
  UICR_MBR_PARAM_PAGE (r)   : ORIGIN = 0x10001018, LENGTH = 4
}

SECTIONS
{
  .fs_data_out ALIGN(4):
  {
    PROVIDE( __start_fs_data = .);
    KEEP(*(fs_data))
    PROVIDE( __stop_fs_data = .);
  } = 0

  .bootloaderSettings(NOLOAD) :
  {
    
  } > BOOTLOADER_SETTINGS

  .uicrBootStartAddress : 
  {
    KEEP(*(.uicrBootStartAddress))
  } > UICR_BOOTLOADER
  
  .mbrParamsPage(NOLOAD) :
  {
    
  } > MBR_PARAMS_PAGE
  
  .uicrMbrParamsPageAddress :
  {
    KEEP(*(.uicrMbrParamsPageAddress))
  } > UICR_MBR_PARAM_PAGE

  .noinit(NOLOAD) :
  {

  } > NOINIT

}

INCLUDE "nrf52_common.ld"
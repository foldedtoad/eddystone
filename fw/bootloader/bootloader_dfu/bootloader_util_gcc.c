/* 
 * Copyright (c) 2015 Robin Callender. All Rights Reserved.
 */
#include <stdint.h>
#include <stdio.h>

#include "bootloader_util.h"
#include "nordic_common.h"
#include "bootloader_types.h"
#include "dfu_types.h"
#include "dbglog.h"

#if __GNUC__ && __ARM_EABI__
/*
 *  Create a union structure which will define the top-most page for
 *  bootloader setting storage, and define the initial default app slot.
 *  This allows a consolidate flash image, including the app, to be 
 *  layed down over all flash, and still not have to provide a 
 *  valid CRC for the initial application image: CRC field is zero.
 *  Subsequenct DFU operations will enforce the CRC check.
 */
typedef struct {
    union {
        bootloader_settings_t init_app;
        uint8_t               asBytes[1024];
    } u;
} flash_area_t;

__attribute__ ((section(".bootloaderSettings"))) flash_area_t  m_boot_settings = {
        .u.init_app.bank_0          = BANK_VALID_APP,  // bank_0 has valid app.
        .u.init_app.bank_0_crc      = 0,               // 0 means don't do CRC check.
        .u.init_app.bank_1          = 0,
        .u.init_app.bank_0_size     = 0,
        .u.init_app.sd_image_size   = 0,
        .u.init_app.bl_image_size   = 0,
        .u.init_app.app_image_size  = 0,
        .u.init_app.sd_image_start  = 0,
};

__attribute__ ((section(".uicrBootStartAddress"))) volatile uint32_t m_uicr_bootloader_start_address = BOOTLOADER_REGION_START;

/* 
 *  Read only pointer to bootloader settings in flash. 
 */
const bootloader_settings_t const * const mp_bootloader_settings = (bootloader_settings_t *) &m_boot_settings;

typedef void (*reset_handler_t)(void);


static inline void StartApplication(uint32_t start_addr)
{ 
    uint32_t reset_vector_table;
    reset_handler_t reset_handler;

    /* Set MSP to the application MSP */
    __set_MSP(*((uint32_t *)start_addr));

    /* Load the address of the reset handler placed after __initial_sp */
    reset_vector_table = *((uint32_t *)(start_addr + 4));
    reset_handler = (void *) reset_vector_table;

    /* Check if application is running in thread mode, and if not exit the current ISR */
    uint32_t isr_vector_num = (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk);

    if (isr_vector_num > 0) {
        __asm volatile( 
            "   LDR   R4, =0xFFFFFFFF       \n"
            "   LDR   R5, =0xFFFFFFFF       \n"
            "   LDR   R6, [R0, #0x00000004] \n"
            "   LDR   R7, =0x21000000       \n"
            "   PUSH  {r4-r7}               \n"
            "   LDR   R4, =0x00000000       \n"
            "   LDR   R5, =0x00000000       \n"
            "   LDR   R6, =0x00000000       \n"
            "   LDR   R7, =0x00000000       \n"
            "   PUSH  {r4-r7}               \n"
            "   LDR   R0, =0xFFFFFFF9       \n"
            "   BX    R0                    \n"
            "   .ALIGN                      \n"
        );
    }

    reset_handler();
}
#endif

void bootloader_util_app_start(uint32_t start_addr)
{
    PRINTF("bootloader_util_app_start: 0x%x\n", (unsigned) start_addr);
    StartApplication(start_addr);
}


void bootloader_util_settings_get(const bootloader_settings_t ** pp_bootloader_settings)
{
    (void) m_uicr_bootloader_start_address;  // Don't remove -- keeps linker happy.

    *pp_bootloader_settings = (bootloader_settings_t *) &m_boot_settings;
}

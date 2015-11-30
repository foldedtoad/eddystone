/* 
 *  Copyright (c) 2015 Robin Callender. All Rights Reserved.
 */
#ifndef PSTORAGE_PL_H__
#define PSTORAGE_PL_H__

#include <stdint.h>

#define PSTORAGE_FLASH_PAGE_SIZE    ((uint16_t)NRF_FICR->CODEPAGESIZE)
#define PSTORAGE_FLASH_EMPTY_MASK    0xFFFFFFFF

#define PSTORAGE_FLASH_PAGE_END                                     \
        ((NRF_UICR->BOOTLOADERADDR != PSTORAGE_FLASH_EMPTY_MASK)    \
        ? (NRF_UICR->BOOTLOADERADDR / PSTORAGE_FLASH_PAGE_SIZE)     \
        : NRF_FICR->CODESIZE)


#define PSTORAGE_MAX_APPLICATIONS   3    
#define PSTORAGE_MIN_BLOCK_SIZE     0x0010

#define PSTORAGE_DATA_START_ADDR    ((PSTORAGE_FLASH_PAGE_END - PSTORAGE_MAX_APPLICATIONS - 1) * PSTORAGE_FLASH_PAGE_SIZE)
#define PSTORAGE_DATA_END_ADDR      ((PSTORAGE_FLASH_PAGE_END - 1) * PSTORAGE_FLASH_PAGE_SIZE) 

#define PSTORAGE_SWAP_ADDR          PSTORAGE_DATA_END_ADDR 

#define PSTORAGE_MAX_BLOCK_SIZE     PSTORAGE_FLASH_PAGE_SIZE  
#define PSTORAGE_CMD_QUEUE_SIZE     30 


/* Abstracts persistently memory block identifier. */
typedef uint32_t pstorage_block_t;

typedef struct {
    uint32_t            module_id;      /* Module ID. */
    pstorage_block_t    block_id;       /* Block ID.  */
} pstorage_handle_t;

typedef uint16_t pstorage_size_t;      /** Size of length and offset fields. */

/*  
 *  Handles Flash Access Result Events. 
 *  To be called in the system event dispatcher of the application. 
 */
void pstorage_sys_event_handler(uint32_t sys_evt);

#endif // PSTORAGE_PL_H__

/*
 *  Copyright (c) 2015 Moment Inc. All Rights Reserved.
 *
 *  This header contains defines with respect persistent storage that are specific to
 *  persistent storage implementation and application use case.
 */
#ifndef PSTORAGE_PL_H__
#define PSTORAGE_PL_H__

#include <stdint.h>

/* 
 *  Size of one flash page. 
 */
#define PSTORAGE_FLASH_PAGE_SIZE    ((uint16_t)NRF_FICR->CODEPAGESIZE)

/* 
 *  Bit mask that defines an empty address in flash.
 */
#define PSTORAGE_FLASH_EMPTY_MASK   0xFFFFFFFF

#define PSTORAGE_FLASH_PAGE_END     NRF_FICR->CODESIZE

/*
 *  Maximum number of applications that can be registered with the module, 
 *  configurable based on system requirements. 
 */
#define PSTORAGE_MAX_APPLICATIONS   1

/*
 *  Minimum size of block that can be registered with the module. 
 *  Should be configured based on system requirements, recommendation is 
 *  not have this value to be at least size of word.
 */
#define PSTORAGE_MIN_BLOCK_SIZE     0x0010


/* 
 *  Start address for persistent data, configurable according to 
 *  system requirements.
 */
#define PSTORAGE_DATA_START_ADDR    ((PSTORAGE_FLASH_PAGE_END - PSTORAGE_MAX_APPLICATIONS) \
                                    * PSTORAGE_FLASH_PAGE_SIZE)

/*
 *  End address for persistent data, configurable according to 
 *  system requirements.
 */
#define PSTORAGE_DATA_END_ADDR      (PSTORAGE_FLASH_PAGE_END * PSTORAGE_FLASH_PAGE_SIZE)

#define PSTORAGE_SWAP_ADDR          PSTORAGE_DATA_END_ADDR

/*  
 *  Maximum size of block that can be registered with the module.
 *  Should be configured based on system requirements. 
 *  And should be greater than or equal to the minimum size.
 */
#define PSTORAGE_MAX_BLOCK_SIZE     PSTORAGE_FLASH_PAGE_SIZE 

/*
 *  Maximum number of flash access commands that can be maintained by the 
 *  module for all applications. Configurable.
 */
#define PSTORAGE_CMD_QUEUE_SIZE     10

/*
 *  Define this flag in case Raw access to persistent memory is to be enabled. 
 *  Raw mode unlike the data mode is for uses other than storing data from various mode. 
 *  This mode is employed when unpdating firmware or similar uses.
 *  Therefore, this mode shall be enabled only for these special use-cases and 
 *  typically disabled.
 */
#define PSTORAGE_RAW_MODE_ENABLE

/* 
 *  Abstracts persistently memory block identifier.
 */
typedef uint32_t pstorage_block_t;

typedef struct
{
    uint32_t            module_id;      /* Module ID.*/
    pstorage_block_t    block_id;       /* Block ID.*/
} pstorage_handle_t;

typedef uint32_t pstorage_size_t;       /* Size of length and offset fields. */

/*
 *  Handles Flash Access Result Events. 
 *  To be called in the system event dispatcher of the application. 
 */
void pstorage_sys_event_handler (uint32_t sys_evt);

#endif // PSTORAGE_PL_H__

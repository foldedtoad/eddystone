/* 
 *  Copyright (c) 2015 Moment. All Rights Reserved.
 *
 *  dfu_init.c
 *
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "dfu_init.h"
#include "dfu_types.h"
#include "nrf_error.h"
#include "crc16.h"
#include "dbglog.h"

#define __breakpoint     __ASM volatile ( "bkpt \n" )

/* 
 *  Minimum length of the extended init packet.
 *  The extended init packet may contain a CRC, a HASH, or other data.
 *  This value must be changed according to the requirements of the system.
 *  The template uses a minimum value of two in order to hold a CRC.
 */
#define DFU_INIT_PACKET_EXT_LENGTH_MIN      2

/* 
 *  Maximum length of the extended init packet. 
 *  The extended init packet may contain a CRC, a HASH, or other data. 
 *  This value must be changed according to the requirements of the system. 
 *  The template uses a maximum value of 10 in order to hold a CRC and any padded 
 *  data on transport layer without overflow.
 */
#define DFU_INIT_PACKET_EXT_LENGTH_MAX      10        

/*
 *  Data array for storage of the extended data received. 
 *  The extended data follows the normal init data of type \ref dfu_init_packet_t. 
 *  Extended data can be used for a CRC, hash, signature, or other data.
 */
static uint8_t m_extended_packet[DFU_INIT_PACKET_EXT_LENGTH_MAX];

/*
 *  Length of the extended data received with init packet.
 */
static uint8_t m_extended_packet_length;                            


uint32_t dfu_init_prevalidate(uint8_t * p_init_data, uint32_t init_data_len)
{
    uint32_t i = 0;
    
    // In order to support signing or encryption then any init packet 
    // decryption function / library should be called from here or 
    // implemented at this location.

    // Length check to ensure valid data are parsed.
    if (init_data_len < sizeof(dfu_init_packet_t)) {
        return NRF_ERROR_INVALID_LENGTH;
    }

    // Current template uses clear text data so they can be casted for pre-check.
    dfu_init_packet_t * p_init_packet = (dfu_init_packet_t *)p_init_data;

    m_extended_packet_length = ((uint32_t)p_init_data + init_data_len) -
                               (uint32_t)&p_init_packet->softdevice[p_init_packet->softdevice_len];

    if (m_extended_packet_length < DFU_INIT_PACKET_EXT_LENGTH_MIN) {
        return NRF_ERROR_INVALID_LENGTH;
    }

    if (((uint32_t)p_init_data + init_data_len) < 
        (uint32_t)&p_init_packet->softdevice[p_init_packet->softdevice_len]) {

        PRINTF("Init data error: 0x%x < 0x%x\n",
            (unsigned) (p_init_data + init_data_len),
            (unsigned) &p_init_packet->softdevice[p_init_packet->softdevice_len]);

        return NRF_ERROR_INVALID_LENGTH;
    }

    memcpy(m_extended_packet,
           &p_init_packet->softdevice[p_init_packet->softdevice_len],
           m_extended_packet_length);

/** [DFU init application version] */
    // In order to support application versioning this check should be updated.
    // This template allows for any application to be installed however customer
    // could place a revision number at bottom of application to be verified by 
    // bootloader. 
    // This could be done at a relative location to this papplication for 
    // example Application start address + 0x0100.
/** [DFU init application version] */
    
    // First check to verify the image to be transfered matches the device type.
    // If no Device type is present in DFU_DEVICE_INFO then any image will be accepted.
    if ((DFU_DEVICE_INFO->device_type != DFU_DEVICE_TYPE_EMPTY) &&
        (p_init_packet->device_type != DFU_DEVICE_INFO->device_type)) {

        return NRF_ERROR_INVALID_DATA;
    }
    
    //
    // Second check to verify the image to be transfered matches the device revision.
    // If no Device revision is present in DFU_DEVICE_INFO then any image will be accepted.
    //
    if ((DFU_DEVICE_INFO->device_rev != DFU_DEVICE_REVISION_EMPTY) &&
        (p_init_packet->device_rev != DFU_DEVICE_INFO->device_rev)) {

        return NRF_ERROR_INVALID_DATA;
    }

    //
    // Third check: Check the array of supported SoftDevices by this application.
    //              If the installed SoftDevice does not match any SoftDevice in 
    //              the list then an error is returned.
    //
    while (i < p_init_packet->softdevice_len) {

        if (p_init_packet->softdevice[i] == DFU_SOFTDEVICE_ANY ||
            p_init_packet->softdevice[i++] == SD_FWID_GET(MBR_SIZE))
        {
            return NRF_SUCCESS;
        }
    }
    
    // No matching SoftDevice found - Return NRF_ERROR_INVALID_DATA.
    return NRF_ERROR_INVALID_DATA;
}


uint32_t dfu_init_postvalidate(uint8_t * p_image, uint32_t image_len)
{
    uint16_t image_crc;
    uint16_t received_crc;
    
    //
    // In order to support hashing (and signing) then the (decrypted) hash 
    // should be fetched and the corresponding hash should be calculated over
    // the image at this location.
    // If hashing (or signing) is added to the system then the CRC validation 
    // should be removed.
    //
    // calculate CRC from active block.
    image_crc = crc16_compute(p_image, image_len, NULL);

    // Decode the received CRC from extended data.    
    received_crc = uint16_decode((uint8_t *)&m_extended_packet[0]);

    // Compare the received and calculated CRC.
    if (image_crc != received_crc) {
        if (received_crc != 0x0000) {
            PRINTF("Bad CRC: image_crc(0x%04x) received_crc(0x%04x)\n",
                   (unsigned)image_crc, (unsigned) received_crc);
        }
        return NRF_ERROR_INVALID_DATA;
    }

    return NRF_SUCCESS;
}


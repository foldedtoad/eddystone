/*---------------------------------------------------------------------------*/
/*  connect.h                                                                */
/*  Copyright (c) 2015 Robin Callender. All Rights Reserved.                 */
/*---------------------------------------------------------------------------*/
#ifndef _CONNECT_H_
#define _CONNECT_H_

#include <stdint.h>

#include "ble_dfu.h"

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

void storage_init(void);
void services_init(void);
void gap_params_init(void);
void conn_params_init(void);
void sec_params_init(void);
void device_manager_init(void);

uint32_t service_changed_indicate(void);

#endif  /* _CONNECT_H_ */

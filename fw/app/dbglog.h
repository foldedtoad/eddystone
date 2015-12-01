/*----------------------------------------------------------------------------*/
/*  dbglog.h   for debug log output                                           */
/*  Copyright (c) 2015 Robin Callender. All Rights Reserved.                  */
/*----------------------------------------------------------------------------*/
#ifndef DBGLOG_H
#define DBGLOG_H

#if defined(PROVISION_DBGLOG)

#include <stdio.h>

#define PRINTF   printf
#define PUTS     puts

#else /* PROVISION_DBGLOG */

#define PRINTF(...)
#define PUTS(s)

#endif /* PROVISION_DBGLOG */

#define TRACE PRINTF("at: %s(%d)\n", __FUNCTION__, __LINE__)

#endif  /* DBGLOG_H */

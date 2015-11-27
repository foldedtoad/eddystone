/*
 *  Copyright (c) 2015 Robin Callender. All Rights Reserved.
 */
#ifndef BOARDS_H
#define BOARDS_H

#if defined (BOARD_PILSY)
  #include "pilsy_board.h"
#elif defined(BOARD_PCA10001)
  #include "pca10001.h"
#elif defined(BOARD_PCA10028)
  #include "pca10028.h"
#else
#error "Board is not defined"
#endif

#endif

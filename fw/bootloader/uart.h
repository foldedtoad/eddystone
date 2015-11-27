/*----------------------------------------------------------------------------*/
/*  uart.h   for debug console output                                         */
/*----------------------------------------------------------------------------*/
#ifndef UART_H
#define UART_H

#include <stdint.h>

void    uart_putc( uint8_t ch );
void    uart_puts( uint8_t * str );
void    uart_init( void );

#endif  /* UART_H */

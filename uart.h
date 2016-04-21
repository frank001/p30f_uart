/* 
 * File:   uart.h
 * Author: frans
 *
 * Created on 19 maart 2016, 10:00
 */

#ifndef UART_H
#define	UART_H

#define FCY   16000000L  //define your instruction frequency, FCY = FOSC/2
#define CYCLES_PER_MS ((unsigned long)(FCY * 0.001))        //instruction cycles per millisecond
#define CYCLES_PER_US ((unsigned long)(FCY * 0.000001))   //instruction cycles per microsecond
#define DELAY_MS(ms)  __delay32(CYCLES_PER_MS * ((unsigned long) ms));   //__delay32 is provided by the compiler, delay some # of milliseconds
 
#define DELAY_US(us)  __delay32(CYCLES_PER_US * ((unsigned long) us));    //delay some number of microseconds
  
#define UART1_BAUD 115200
#define UBRG1_VALUE (FCY/UART1_BAUD)/16 - 1

void initUART(void);
void Commit(void);


#endif	/* UART_H */


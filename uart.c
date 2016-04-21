#include <p30f4013.h>
#include "uart.h"
#include "websocket.h"

unsigned char rxBuffer[MAXBYTES];
unsigned char rxPtr = 0;

unsigned char txBuffer[MAXBYTES];
unsigned char txPtr = 0;
extern unsigned char writePtr;

void initUART(void) {       //initialize UART1
    _TRISF3 = 0;            //UART1 TX
    _TRISF2 = 1;            //UART1 RX
    U1BRG = UBRG1_VALUE;
    IEC0bits.U1RXIE=1;
    U1STA&=0xfffc;
    U1MODEbits.UARTEN=1;
    U1STAbits.UTXEN=1;
 }

void Commit(void) {
    IEC0bits.U1TXIE=1;                //enable TX interrupt
}

void __attribute__((__interrupt__, __no_auto_psv__)) _U1RXInterrupt(void) {
        rxBuffer[rxPtr++] = U1RXREG;
        rxPtr%=MAXBYTES;
        IFS0bits.U1RXIF = 0;        //clear interrupt flag
}

void __attribute__((__interrupt__, __no_auto_psv__)) _U1TXInterrupt(void) {
    U1TXREG = txBuffer[txPtr++];
    txPtr%=MAXBYTES;
    while (!U1STAbits.TRMT) ;
    if (txPtr==writePtr) IEC0bits.U1TXIE=0;     //finished writing, disable TX interrupt
}

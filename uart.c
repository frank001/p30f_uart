#include <p30f4013.h>
#include "uart.h"
#include "websocket.h"

unsigned char rxBuffer[MAXBYTES];
unsigned char rxPtr = 0;

unsigned char txBuffer[MAXBYTES];
unsigned char txPtr = 0;
extern unsigned char writePtr;

void initUART(void) {
    //initialize UART2
    _TRISF5 = 0;        //UART2 TX
    _TRISF4 = 1;        //UART2 RX
    U2BRG = UBRG2_VALUE;
    IEC1bits.U2RXIE=1;
    IEC1bits.U2TXIE=1;
    U2STA&=0xfffc;
    U2MODEbits.UARTEN=1;
    U2STAbits.UTXEN=1;
}

void Commit(void) {
    while (IFS1bits.U2TXIF) ;       //wait until clear to transmit
    IFS1bits.U2TXIF = 1;            //raise interrupt, the routine will take care of the rest
}

void __attribute__((__interrupt__, __no_auto_psv__)) _U2RXInterrupt(void) {
        rxBuffer[rxPtr++] = U2RXREG;
        rxPtr%=MAXBYTES;
        IFS1bits.U2RXIF = 0;        //clear interrupt flag
}

void __attribute__((__interrupt__, __no_auto_psv__)) _U2TXInterrupt(void) {
        U2TXREG = txBuffer[txPtr++];
        txPtr%=MAXBYTES;
        while (!U2STAbits.TRMT) ;
        if (txPtr==writePtr) IFS1bits.U2TXIF = 0;    //clear interrupt flag
}

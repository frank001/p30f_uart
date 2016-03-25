#include <p30f4013.h>
#include "interrupt.h"
#include "websocket.h"

unsigned char rxBuffer[MAXBYTES];
unsigned char rxPtr = 0;

unsigned char txBuffer[MAXBYTES];
unsigned char txPtr = 0;
extern unsigned char writePtr;

unsigned char tempRX2;
unsigned char byteCount = 0;


void __attribute__((__interrupt__, __auto_psv__)) _U2RXInterrupt(void) {
    tempRX2 = U2RXREG;
    rxBuffer[rxPtr++] = tempRX2;
    rxPtr%=MAXBYTES;
    IFS1bits.U2RXIF = 0;        //clear interrupt flag
}

void __attribute__((__interrupt__, __auto_psv__)) _U2TXInterrupt(void) {
    while (txPtr!=writePtr) {
        U2TXREG = txBuffer[txPtr++];
        while (!U2STAbits.TRMT) ;
        txPtr%=MAXBYTES;
    }
    IFS1bits.U2TXIF = 0;    //clear interrupt flag
    
}

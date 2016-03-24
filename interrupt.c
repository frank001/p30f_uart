#include <p30f4013.h>
#include "interrupt.h"
#include "websocket.h"

unsigned char buffer[255];
unsigned char tempRX2;
unsigned char byteCount =0;

void __attribute__((__interrupt__, __auto_psv__)) _U2RXInterrupt(void) {
    tempRX2 = U2RXREG;
    ReadClient(tempRX2);
    //buffer[byteCount++] = tempRX2;
    IFS1bits.U2RXIF = 0;
}

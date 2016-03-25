#include <stdio.h>
#include <stdlib.h>
#include <p30f4013.h>
#include <uart.h>
#include "config.h"
#include "interrupt.h"

extern unsigned char txBuffer[];
extern unsigned char txPtr;

unsigned char isConnected = 0;

unsigned char hello[] ="Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!";


void delay(unsigned int d) {
    __delay32(d*10000);
}


int main(void) {
    //TRISB = 0x10;
    int i;
    int timeOut;
    char flipbit;
    _TRISF5 = 0;        //UART2 TX
    _TRISF4 = 1;        //UART2 RX
    
    TRISB = 0;
    _LATD2 = 1;
    _TRISD2 = 1;
    
    
    unsigned char tx;
    
    U2BRG = UBRG2_VALUE;
    
    IEC1bits.U2RXIE=1;
    IEC1bits.U2TXIE=1;
    U2STA&=0xfffc;
    U2MODEbits.UARTEN=1;
    U2STAbits.UTXEN=1;
    
    
    while (1) {
        
        if (_RD2==0 ) {
            if (!flipbit) DELAY_MS(100);
            if (_RD2==0) {
                flipbit = 1;
                                                //TODO: don't send when not connected!
                while (IFS1bits.U2TXIF) ;       //wait until clear to transmit
                //IEC1bits.U2TXIE=0;
                WriteWebSocket(0x81);           //FIN bit high and opcode=1
                WriteWebSocket(sizeof(hello));  //payload length
                for (i=0;i<sizeof(hello);i++) 
                    WriteWebSocket(hello[i]);
                //IEC1bits.U2TXIE=1;
                U2TXREG = txBuffer[txPtr++];    //write the first byte
                
                _LATB3 = 1;
            }
        }
        else { 
            _LATB3 = 0;
            flipbit=0;
        }
        DELAY_MS(15);
        ReadWebSocket();    
        
        //_LATB0 = 1;
        //DELAY_MS(1000);
        //_LATB0 = 0;
        //DELAY_MS(1000);
        
    }
    
}

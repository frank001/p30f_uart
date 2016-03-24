#include <stdio.h>
#include <stdlib.h>
#include <p30f4013.h>
#include <uart.h>
#include "config.h"
#include "interrupt.h"



unsigned char resetDevice = 0;
unsigned char isConnected = 0;
unsigned char txUART[255];
unsigned char txCount;

unsigned char hello[] ="Hello World!";


void delay(unsigned int d) {
    __delay32(d*10000);
}

void writeUART(unsigned char trx) {
    U2TXREG = trx;
    while (!U2STAbits.TRMT) ;
    
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
    U2STA&=0xfffc;
    U2MODEbits.UARTEN=1;
    U2STAbits.UTXEN=1;
    
    while (1) {
        
        if (_RD2==0 ) {
            if (!flipbit) DELAY_MS(100);
            if (_RD2==0) {
                flipbit = 1;
                IEC1bits.U2RXIE=0;              //disable receive interrupt.
                                                //TODO: don't send when not connected!
                //DELAY_MS(10);
                _LATB3 = 1;
                if (txCount == 0) {
   //                 txCount = 0;                    //we're gonna write, so set the pointer to zero
                    txUART[txCount++] = 0x81;       //FIN bit high and opcode=1
                    txUART[txCount++] = sizeof(hello);
                    for (tx=0;tx<sizeof(hello);tx++) txUART[txCount++] = hello[tx];
                    //txCount--;
                    //IEC1bits.U2RXIE=1;              
                }
                //}
            }
        }
        else { 
            _LATB3 = 0;
            flipbit=0;
        }
            //DELAY_MS(100);
            
        
        if (txUART[0]!=0) {    //we have data to transmit to UART.
            for (i=0;i<txCount;i++) {
                tx = txUART[i];
                writeUART(tx);
            }
            txCount = 0;
            for (i=0;i<255;i++) txUART[i]=0;
            
        }
        IEC1bits.U2RXIE=1; //enable receive interrupt
        
        //_LATB0 = 1;
        //DELAY_MS(1000);
        //_LATB0 = 0;
        //DELAY_MS(1000);
        
    }
    
}

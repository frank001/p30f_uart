#include <stdio.h>
#include <stdlib.h>
#include <p30f4013.h>
#include <uart.h>
#include "config.h"
#include "uart.h"
#include "timers.h"
#include "adc.h"

extern unsigned char txBuffer[];
extern unsigned char txPtr;

unsigned char isConnected = 0;



//unsigned char hello[] ="Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!Hello World!!!!";
unsigned char hello[] ="Hello World!Hello World!Hello W";
extern unsigned int ADC10;

void delay(unsigned int d) {
    __delay32(d*10000);
}


int main(void) {
    int i;
    char flipbit=0;
    char senddata=0;
    unsigned int adc10;
    

    TRISB = 0;
    
    _TRISB10 = 1;
    
    
    
    _LATD2 = 1;
    _TRISD2 = 1;
    
    initADC();
    initUART();
    
    initTimer4();
    startTimer4();
    
    adc10 = ADC10;
    while (1) {
        if (!flipbit && _RD2==0) {
            DELAY_MS(100);
            if (_RD2==0) { 
                senddata=!senddata;
                flipbit = 1;
            }
        }
        if (flipbit && _RD2==1) {
            DELAY_MS(100);
            if (_RD2==1) flipbit = 0;
        }
        _LATB3 = senddata;        

        if (isConnected) {
            if (adc10!=ADC10) {
                adc10 = ADC10;
                WriteWebSocket(0x82);           //FIN bit high and opcode=2 (binary)
                WriteWebSocket(0x02);  //payload length
                //for (i=0;i<sizeof(adc10);i++) 
                WriteWebSocket(adc10 & 0x00ff);
                WriteWebSocket((adc10 & 0xff00)>>7);
                Commit();
            }
            
            if (senddata) {
                WriteWebSocket(0x81);           //FIN bit high and opcode=1
                WriteWebSocket(sizeof(hello));  //payload length
                for (i=0;i<sizeof(hello);i++) 
                    WriteWebSocket(hello[i]);
                Commit();
                //_LATB3 = 1;
            }
            /*else { 
                //_LATB3 = 1;
                if (flipbit) DELAY_MS(100);
                if (_RD2==1){
                    flipbit=0;
                    _LATB3 = 0;
                }
            }*/
        }
        DELAY_MS(10);
        ReadWebSocket();    
    }
    
}

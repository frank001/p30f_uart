#include <p30f4013.h>
#include "timers.h"

unsigned int  TimeSlice = 0;

// Timer4 is used for a heartbeat LED indication and for delay routine
// T4CON:	B15=TON B13=TSIDL B5=TGATE B5:4=TCKPS1:0 B3=T32 B1=TCS


void initTimer4(void) {
	stopTimer4();
    T4CONbits.TCS = 0;
    IEC1bits.T4IE = 1;
    IFS1bits.T4IF = 0;
	PR4 = 6250;	//period 100ms
}

void startTimer4(void) {
	//Fosc runs at 8*8 MHz, timer source = 64/4 = 16 MHz
	//prescaler = 1:256 -> 62500 kHz timer clock
	T4CON = 0x8030;			// Start timer4
}

void stopTimer4(void) {
	T4CON = 0x0000;			// Stop timer4
	TMR4  = 0;				// Clear timer register
}


void __attribute__((__interrupt__,__no_auto_psv__)) _T4Interrupt( void ) {
	static unsigned char ledCount = 0;
	// Clear interrupt flag
	_T4IF = 0;
	// Increment time slice counter
	TimeSlice++;
	// Show alive led
	if ((ledCount == 1) || (ledCount == 3))
		_LATB4 = 1;
	else
		_LATB4 = 0;
	ledCount++;
	if (ledCount == 10)
		ledCount = 0;
}

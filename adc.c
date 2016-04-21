#include <p30f4013.h>


unsigned int ADC10 = 0;
unsigned char adcCount = 0;
unsigned int samples = 0xff;
unsigned long adcSum=0;

void initADC(void) {
    //_LATB10 = 1;
    //ADPCFG = 0xfbff;       //tenth channel for ADC
    //ADPCFG = 0xfbff;
    ADCON1 = 0x0004;
    ADCHS = 0x000a;         //connect RB10 to AN10 as CH0 input
    ADCSSL = 0;             //no scan
    ADCON3 = 0x1003;        //ADCS=3 (min TAD for 10MHz is 3*TCY=300ns)
    ADCON2 = 0;             //Interrupt upon completion of one sample/convert
    //ADCON1.F15 = 1;         //ADC on
    
    ADCON1 = 0;
    ADCON1bits.SSRC = 7;
    ADCON1bits.ASAM = 1;
    ADCON1bits.FORM = 0;
    
    ADCON2bits.SMPI = 6;
    ADCON2bits.VCFG = 0;
    ADCON2bits.CSCNA = 1;
    
    ADCON3bits.SAMC = 31; 	
    ADCON3bits.ADRC = 1;
    ADCON3bits.ADCS = 20;
    
    ADCHS = 0xffff;         //don't care?
    
    ADCSSLbits.CSSL10 = 1;
    ADPCFGbits.PCFG10 = 0;
    
    IFS0bits.ADIF = 0;      //clear interrupt flag
    
    IEC0bits.ADIE = 1;
    IEC0bits.ADIE = 1;		// interrupt enable

	ADCON1bits.ADON = 1;    //start sampling
    
}

void __attribute__((__interrupt__,no_auto_psv)) _ADCInterrupt(void)  { 
    unsigned int adcValue;
    unsigned int mean;
    
    adcCount++;
    adcSum+=ADCBUF0;
    if (adcCount==samples){
        mean = adcSum / samples;
        adcCount = 0;
        adcSum=0;
        if (ADC10<(mean-0x0f) || ADC10>(mean+0x0f)) ADC10=mean;
        
    }
    _ADIF = 0; 
 
}


/* 
 * File:   config.h
 * Author: frans
 *
 * Created on 18 maart 2016, 19:27
 */

#ifndef CONFIG_H
#define	CONFIG_H


 int FOSC __attribute__((space(prog), address(0xF80000))) = 0xC70A ;    //16MHz? not sure, fz

 int FWDT __attribute__((space(prog), address(0xF80002))) = 0x3F ;
//_FWDT(
//    WDTPSB_16 &        // WDT Prescaler B (1:16)
//    WDTPSA_512 &       // WDT Prescaler A (1:512)
//    WDT_OFF            // Watchdog Timer (Disabled)
//);


#endif	/* CONFIG_H */


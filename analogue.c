/*
 Routines for CBUS FLiM operations - part of CBUS libraries for PIC 18F
  This work is licensed under the:
      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
   To view a copy of this license, visit:
      http://creativecommons.org/licenses/by-nc-sa/4.0/
   or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
   License summary:
    You are free to:
      Share, copy and redistribute the material in any medium or format
      Adapt, remix, transform, and build upon the material
    The licensor cannot revoke these freedoms as long as you follow the license terms.
    Attribution : You must give appropriate credit, provide a link to the license,
                   and indicate if changes were made. You may do so in any reasonable manner,
                   but not in any way that suggests the licensor endorses you or your use.
    NonCommercial : You may not use the material for commercial purposes. **(see note below)
    ShareAlike : If you remix, transform, or build upon the material, you must distribute
                  your contributions under the same license as the original.
    No additional restrictions : You may not apply legal terms or technological measures that
                                  legally restrict others from doing anything the license permits.
   ** For commercial use, please contact the original copyright holder(s) to agree licensing terms
**************************************************************************************************************
	The FLiM routines have no code or definitions that are specific to any
	module, so they can be used to provide FLiM facilities for any module 
	using these libraries.
	
*/ 
/* 
 * File:   analogue.c
 * Author: Ian
 *
 * Created on 16 August 2017, 13:14
 *
 * Process the analogue inputs (include magnet)
 */
#include "module.h"
#include "analogue.h"
#include "cbus.h"
#include "romops.h"
#include "potentiometer.h"

unsigned char lastReading;

void initAnalogue(unsigned char port) {
    ANCON0 = 1 << port; // make it an analogue port 
    ANCON1 = 0;
    TRISAbits.TRISA5 = 1;   // Input
    
    /* Single channel measurement mode */
        
    ADCON0 = (port << 2) | 0x01; // select the input channel and turn on ADC
    ADCON1 = 0;                 // Single channel measurement mode between AVss and AVcc
    ADCON2 = 0x16;              // Acquisition 4 Tad cycles and Fosc/64. 
                                // Left justified result so that 8 bit result is in ADRESH

    ADCON0bits.ADON = 1;      // turn on ADC module
    // start an ADC
    ADCON0bits.GO = 1;
    // wait for result
    while (ADCON0bits.GO)
           ;
    // get the reading
    lastReading = ADRESH;
}


void pollAnalogue(unsigned char port) {
    unsigned char io;
    unsigned char adc;

    // is conversion finished?
    if ( ! ADCON0bits.GO) {
        // get the 8 bit result
        adc = ADRESH;

        //save
        lastReading = adc;
        // start another conversion
        ADCON0bits.GO = 1;
    }
}


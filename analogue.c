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

WORD lastReading;

void initAnalogue(unsigned char port) {
    ADCON1bits.VCFG = 0;
    ADCON1bits.VNCFG = 0;
    ADCON1bits.TRIGSEL = 0;
    ADCON1bits.CHSN = 0;
    ADCON2bits.ADFM = 1;    // right justified
    ADCON2bits.ACQT = 2;    // Acquisition 4 Tad cycles
    ADCON2bits.ADCS = 6;    // Fosc/64

    ADCON0bits.ADON = 1;

    // start an ADC
    ADCON0bits.CHS = port;
    ADCON0bits.GO = 1;
    // wait for result
    while (ADCON0bits.GO)
           ;
    // get the reading
    lastReading = ADRESH;
    lastReading << 8;
    lastReading |= ADRESL;
    lastReading >> 4;
}


void pollAnalogue(unsigned char port) {
    unsigned char io;
    WORD adc;

    // is conversion finished?
    if ( ! ADCON0bits.GO) {
        // get the 12 bit result
        adc = ADRESH;
        adc = adc << 8;
        adc |= ADRESL;
        adc >> 4;   // convert to 8 bit

        //save
        lastReading = adc;
    } else {
        // start a conversion
        ADCON0bits.CHS = port;
        ADCON0bits.GO = 1;
    }
}


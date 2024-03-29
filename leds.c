/*
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
*/ 
/* 
 * File:   leds.c
 * Author: Ian
 * 
 * Handle a CANPAN led matrix.
 *
 * Created on 7 Mar 2020
 */

#include <stddef.h>
#include "devincs.h"
#include "cabdcNv.h"
#include "candccab.h"
#include "cbus.h"

// RB4 - RB7 are used to drive the LED Anodes
// MSSP SSI Master is used to provide 8 bits for cathodes

static unsigned char current_row = 0;
unsigned char led_matrix[4];

/**
 * Turn on an LED. No is 0-31.
 * @param no
 */
void setLed(unsigned char no) {
    led_matrix[no/8] |= (1 << (no%8));
}

/** 
 * Turn an LED off. No is 0-31
 */
void clearLed(unsigned char no) {
    led_matrix[no/8] &= ~(1 << (no%8));
}

/**
 * Test if an LED is on. 
 * @param no
 * @return 0 if OFF or non zero if ON
 */
unsigned char testLed(unsigned char no) {
    return led_matrix[no/8] & (1 << (no%8));
}

void initLeds() {
    unsigned char i;
    for (i=0; i<4; i++) {
        led_matrix[i] = 0;
    }
    TRISC = 0x80;   // RC7 is the CAN Rx
    LATB = 0xF0;    // LED drivers off
    TRISB = 0xf;    // upper 4 bits are outputs, lower 4 are inputs
    //Set up the MSSP to drive the switch matrix
    SSPCON1 = 0x22; // Enable Master and clock for Fosc/64
    SSPSTATbits.CKE = 1;
}

/**
 * Called every 2ms to load the next row of the LED matrix
 */
void pollLeds() {
    unsigned char dummy;
    unsigned char anodes;
    unsigned char cathodes;
    
    current_row++;
    current_row &= 0x3;
    // turn off the cathode drivers
    LATCbits.LATC2 = 1; // OE
    
    PIR1bits.SSPIF = 0; // clear the flag ready for next time
    dummy = SSPBUF; // dummy read needed before next write
    //SSPCON1bits.WCOL = 0;
    cathodes = led_matrix[current_row];
    SSPCON1 = 0x22;
    PIR1bits.SSPIF = 0; // clear the flag ready for next time
    SSPBUF = cathodes;
    
//    // wait for data to be sent
    // This takes quite a bit of time but we have to ensure the cathodes have the
    // right data before turning on the anodes otherwise we don't get a clean display.
    while (PIR1bits.SSPIF == 0)
        ;

    // latch the data
    LATCbits.LATC4 = 1; //LE
    LATCbits.LATC4 = 0; //LE
    // turn the relevant anode driver on
    anodes = ~(1 << 4+current_row);
    LATB = anodes & 0xf0;
    // turn the relevant cathode driver back on
    LATCbits.LATC2 = 0; //OE
}

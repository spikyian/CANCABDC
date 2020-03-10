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
 * File:   switches.c
 * Author: Ian
 * 
 * Handle a CANPAN switch matrix.
 * Note this this does not debounce the switches as there is no need in the 
 * CANCABDC application.
 *
 * Created on 7 Mar 2020
 */

#include <stddef.h>
#include "devincs.h"
#include "cabdcNv.h"
#include "candccab.h"
#include "cbus.h"
#include "sections.h"


// PIN configs
// RA0-RA2 are used to indicate which of the 8 rows is being scanned.
// RB0-RB4 are the switch column bits.

static unsigned char scan_column;
unsigned char switch_matrix[8]; // lower 4 bits are used

#define DEBOUNCE    4
unsigned char debounce[8][4];

void initSwitches(void) {
    unsigned char i;
    for (i=0; i<8; i++) {
        switch_matrix[i] = 0;
        debounce[i][0] = 0;
        debounce[i][1] = 0;
        debounce[i][2] = 0;
        debounce[i][3] = 0;
    }
    //Set up the IO ports to be able to read the switch matrix
    TRISA = 0x7;  // RA0-RA2 are outputs
    TRISB = 0xf;    // upper 4 bits are outputs, lower 4 are inputs
}

void pollSwitches(void) {
    unsigned char col;
    unsigned char diffs;
    unsigned char i;
    // read the current column
    col = (PORTB & 0xf);
    // check if there are any changes
    diffs = col^ switch_matrix[scan_column];
    // go through each of the 4 col bits
    for (i=0; i<4; i++) {
        // check if we are still in a debounce period
        if (debounce[scan_column][i] >0) {
            debounce[scan_column][i]--;
        } else {
            // check if the bit has changed
            unsigned char bit = (1<<i);
            if (diffs & bit) {
                //change after the debounce time
                
                switch_matrix[scan_column] &= ~bit;
                switch_matrix[scan_column] |= bit;
                // set the debounce timer again
                debounce[scan_column][i] = DEBOUNCE;
                // call the section state machine
                switch_pressed(i*8 + scan_column);
            }
        }
    }
    
    // get ready for next row.
    scan_column++;
    scan_column &=0x7;
    LATA = scan_column;
}
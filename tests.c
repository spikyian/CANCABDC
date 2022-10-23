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
 * File:   tests.c
 * Author: Ian
 * 
 * Test a CANPAN LED/switch matrix.
 *
 * Created on 26 Mar 2020
 */

#include <stddef.h>
#include "devincs.h"
#include "TickTime.h"
#include "switches.h"
#include "leds.h"
#include "analogue.h"
#include "statusLeds.h"

extern TickValue startTime;
extern TickValue lastSwitchPollTime;
extern TickValue lastLedPollTime;
extern TickValue lastAnaloguePollTime;


#define TEST_COL1       1
#define TEST_COL2       2
#define TEST_COL3       3
#define TEST_COL4       4
#define TEST_ROW1       5
#define TEST_ROW2       6
#define TEST_ROW3       7
#define TEST_ROW4       8    
#define TEST_ROW5       9    
#define TEST_ROW6       10    
#define TEST_ROW7       11   
#define TEST_ROW8       12
#define TEST_SWITCHES   13
#define TEST_REPEAT     24  // 10 seconds doing TEST_SWITCHES

/**
 * This test goes through a sequence of lighting each column and then each row
 * followed by lighting the respective LED when a switch is pressed. 
 */
void test1(void) {
    unsigned char testStep = TEST_COL1;
    TickValue testTime;
    testTime.Val = startTime.Val;
        
    while (TRUE) {
        unsigned char i;

        if (testStep >= TEST_SWITCHES) {    
            // just copy switches to leds
            for (i=0; i<4; i++) {
                led_matrix[i] = (switch_matrix[2*i] & 0xF) | (switch_matrix[2*i+1]<<4);
            }
        } 
        if (tickTimeSince(testTime) > (ONE_SECOND)) {
            if ((testStep >= TEST_COL1) && (testStep <= TEST_COL4)) {
                // illuminate the cols in turn
                for (i=0; i<4; i++) {
                    if (i == (testStep-TEST_COL1)) {
                        led_matrix[i] = 0xff;
                    } else {
                        led_matrix[i] = 0;
                    }
                }
                
            } 
            if ((testStep >= TEST_ROW1) && (testStep <= TEST_ROW8)) {
                // illuminate the rows in turn
                for (i=0; i<4; i++) {
                    led_matrix[i] = (1 << (testStep - TEST_ROW1));
                }
            }
            testStep++;
            testTime.Val = tickGet();
            if (testStep >= TEST_REPEAT) {  // restart the tests
                testStep = TEST_COL1;
            }
        }
        if (tickTimeSince(lastSwitchPollTime) > (2 * ONE_MILI_SECOND)) {
            pollSwitches(0);
            lastSwitchPollTime.Val = tickGet();
        }
        if (tickTimeSince(lastLedPollTime) > (2 * ONE_MILI_SECOND)) {
            pollLeds();
            lastLedPollTime.Val = tickGet();
        }
        checkFlashing();
    }        
}

/**
 * This test goes through a sequence of lighting each LED in turn. 
 */
void test2(void) {
    unsigned char led = 31;
    TickValue testTime;
    testTime.Val = startTime.Val;
        
    while (TRUE) {
        if (tickTimeSince(testTime) > (ONE_SECOND)) {
            clearLed(led);
            led++;
            if (led >=32) led=0;
            setLed(led);
            testTime.Val = tickGet();
        }
        if (tickTimeSince(lastLedPollTime) > (2 * ONE_MILI_SECOND)) {
            pollLeds();
            lastLedPollTime.Val = tickGet();
        }
        checkFlashing();
    }        
}

#define ANALOGUE_PORT 4
/**
 * This test reads the analogue value and moved the lit LED accordingly,
 */
void test3(void) {
    unsigned char led = 0;
    TickValue testTime;
    testTime.Val = startTime.Val;
        
    while (TRUE) {
        if (tickTimeSince(lastAnaloguePollTime) > (6 * ONE_MILI_SECOND)) {
            pollAnalogue(ANALOGUE_PORT);
            lastAnaloguePollTime.Val = tickGet();
        }
        if (tickTimeSince(testTime) > (19 * ONE_MILI_SECOND)) { 
            clearLed(led);
            // Analogue lastReading value is 8 bit. Convert to 5 bit (0-31). 
            led = (lastReading >> 3) & 0x1F;
            setLed(led);
            testTime.Val = tickGet();
        }
        if (tickTimeSince(lastLedPollTime) > (2 * ONE_MILI_SECOND)) {
            pollLeds();
            lastLedPollTime.Val = tickGet();
        }
        checkFlashing();
    }
}


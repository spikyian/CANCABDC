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

#include <stddef.h>
#include "devincs.h"
#include "cabdcNv.h"
#include "candccab.h"
#include "cbus.h"

#include "leds.h"
#include "potentiometer.h"
#include "sections.h"
#include "cabdcNv.h"
#include "nvCache.h"
#include "analogue.h"
#include "switches.h"

unsigned char previousReading;
char previousSpeed;

// Forward declarations
void setSpeed(unsigned char section, char speed);
void setAllSpeed(char speed);

/**
 *  Call this after initAnalogue()
 */
void initPotentiometer() {
    previousReading = lastReading;
    previousSpeed = 0;
}

int abs(int a) {
    return (a < 0) ? -a : a;
}
int sgn(int a) {
    return (a < 0) ? -1 : 1; 
}

/**
 * Convert a reading to a speed value.
 * 
 *                        speed
 *                          ^
 *                          |
 *                          C                /----------
 *                          |              /
 *                          |            /
 *                          |          /
 *                          |        /
 *                          B       |
 *                          |       |
 * ---------0-------A-------+-------A---------255---------> reading
 *                  |       | 
 *                  |       B 
 *                 /        |
 *               /          |
 *             /            |
 *           /              |
 * ----------               C
 *                          |
 *                          |
 * 
 * A = pot_dead_zone
 * B = pot_start_level
 * C = pot_end_level
 * @param reading (8 bit)
 * @return speed (8 bit -128 to +127)
 */
char speed(unsigned char reading) {
    int r = reading-128;
    char sign;
    if (abs(r) < NV->pot_dead_zone) return 0;
    
    // now the linear bit
    sign = sgn(r);
    // (127-A)speed =  (reading-A)(C-B) + (127-A)B
//    signedReading -= NV->pot_dead_zone;
    r = abs(r) - NV->pot_dead_zone;
    r *= (NV->pot_end_level - NV->pot_start_level);
    r = NV->pot_start_level + r/(128 - NV->pot_dead_zone);
    return sign*r;
}

/** 
 * Call this regularly at the maximum rate of transmitting the speed changes.
 * You also need to call pollAnalogue() to ensure you get a recent pot setting
 */
void pollPotentiometer(void) {
    char currentSpeed;
    if (previousReading != lastReading) {
        // pot has changed
        previousReading = lastReading;
        
        currentSpeed = speed(lastReading);
        if (previousSpeed != currentSpeed) { 
            previousSpeed = currentSpeed;
            setAllSpeed(currentSpeed);
        }
    }
}

void setAllSpeed(char speed) {
    unsigned char i;
    
    for (i=0; i<NUM_SECTIONS; i++) {
        if (testLed(sections[i].ourControl_led)) {
            if (getSwitchState(sections[i].direction_switch)) {
                setSpeed(i, -speed);
            } else {
                setSpeed(i, speed);
            }
        }
    }
}

/**
 * See documentation on CAN4DC for the encoding of the speed control events,
 * @param section
 * @param speed
 */
void setSpeed(unsigned char section, char speed) {
    unsigned short nn, en;
    cbusMsg[d5] = speed;
    cbusMsg[d6] = NV->acceleration;
    if (NV->frequency) {
        cbusMsg[d6] |= 0x80;
    } else {
        cbusMsg[d6] &= 0x7F;
    }
    cbusMsg[d7] = 0;

    if ((NV->sections[section].section_nn_bytes.section_nn_h != 0) || (NV->sections[section].section_nn_bytes.section_nn_l != 0)) {
        // This should be a ACON3
        nn = NV->sections[section].section_nn_bytes.section_nn_h;
        nn <<= 8;
        nn |= NV->sections[section].section_nn_bytes.section_nn_l;
        en = NV->sections[section].section_en_bytes.section_en_h;
        en <<= 8;
        en |= NV->sections[section].section_en_bytes.section_en_l;
        cbusSendEventWithData( CBUS_OVER_CAN, nn, en, 1, cbusMsg, 3);
    }
}



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
#include "happeningsActions.h"

#include "leds.h"
#include "sections.h"
#include "switches.h"
#include "potentiometer.h"
#include "cabdcNv.h"
#include "FliM.h"
#include "nvCache.h"

/* 
 * File:   sections.c
 * Author: Ian
 * 
 * Handle the single controller messages and state machine.
 * Created on 7 Mar 2020
 * 
 *                         requestControl
 *  haveControl=false  ---------------------->  haveControl=true
 *  controlled=false  <-----------------------  controlled=true
 *        |  ^             releaseControl       /
 *        |  |                                 /
 *        |  |                                /
 *        |  |                               /
 *     Got|  |lostControl                   /Got
 * control|  |message                      /control
 * message|  |                            /message
 *        |  |                           /
 *        v  |                          /
 *  haveControl=false  <---------------/
 *  controlled=true
 *
 * 
 * Use the LEDs to store state:
 * LEDs:
 * have control controlled  meaning
 *  0           0           Section is off, uncontrolled
 *  0           1           Some other panel is controlling this section
 *  1           0           unused
 *  1           1           This panel is controlling this section
 */

// The LEDs and Switches are numbered in different ways

// request_switch, release_switch, haveControl_led, controlled_led

Section sections[NUM_SECTIONS]; 
static unsigned char switch2Section[NUM_SWITCHES];

/**
 * 
 */
void initSections(void) {
    unsigned char i;
/*   
    sections[0].request_switch = 0;
    sections[0].release_switch = 16;
    sections[0].controlled_led = 0;
    sections[0].haveControl_led = 8;
    
    sections[1].request_switch = 1;
    sections[1].release_switch = 17;
    sections[1].controlled_led = 1;
    sections[1].haveControl_led = 9;
   
    sections[2].request_switch = 2;
    sections[2].release_switch = 18;
    sections[2].controlled_led = 2;
    sections[2].haveControl_led = 10;
    
    sections[3].request_switch = 3;
    sections[3].release_switch = 19;
    sections[3].controlled_led = 3;
    sections[3].haveControl_led = 11;
    
    sections[4].request_switch = 4;
    sections[4].release_switch = 20;
    sections[4].controlled_led = 4;
    sections[4].haveControl_led = 12;
    
    sections[5].request_switch = 5;
    sections[5].release_switch = 21;
    sections[5].controlled_led = 5;
    sections[5].haveControl_led = 13;
    
    sections[6].request_switch = 6;
    sections[6].release_switch = 22;
    sections[6].controlled_led = 6;
    sections[6].haveControl_led = 14;
    
    sections[7].request_switch = 7;
    sections[7].release_switch = 23;
    sections[7].controlled_led = 7;
    sections[7].haveControl_led = 15;
    
    sections[8].request_switch = 8;
    sections[8].release_switch = 24;
    sections[8].controlled_led = 16;
    sections[8].haveControl_led = 24;
    
    sections[9].request_switch = 9;
    sections[9].release_switch = 25;
    sections[9].controlled_led = 17;
    sections[9].haveControl_led = 25;
    
    sections[10].request_switch = 10;
    sections[10].release_switch = 26;
    sections[10].controlled_led = 18;
    sections[10].haveControl_led = 26;
    
    sections[11].request_switch = 11;
    sections[11].release_switch = 27;
    sections[11].controlled_led = 19;
    sections[11].haveControl_led = 27;
    
    sections[12].request_switch = 12;
    sections[12].release_switch = 28;
    sections[12].controlled_led = 20;
    sections[12].haveControl_led = 28;
    
    sections[13].request_switch = 13;
    sections[13].release_switch = 29;
    sections[13].controlled_led = 21;
    sections[13].haveControl_led = 29;
    
    sections[14].request_switch = 14;
    sections[14].release_switch = 30;
    sections[14].controlled_led = 22;
    sections[14].haveControl_led = 30;
    
    sections[15].request_switch = 15;
    sections[15].release_switch = 31;
    sections[15].controlled_led = 23;
    sections[15].haveControl_led = 31;
*/
      
    // fill in the switch2Section lookup table
    for (i=0; i<NUM_SECTIONS; i++){
        sections[i].request_switch = i;
        sections[i].direction_switch = (i+16);
        sections[i].otherControlled_led = i+(i/8)*8;
        sections[i].ourControl_led = 8+i+(i/8)*8;
        switch2Section[sections[i].request_switch] = i;
        switch2Section[sections[i].direction_switch] = i;
    }
}

/**
 * A switch has changed state (pressed or released). This drives the section 
 * control state machine.
 * 
 * @param sw
 * @param state
 */
void switch_pressed(unsigned char sw, unsigned char state) {
    unsigned char section;
    
    if (state == 0) {
        // we are only interested in switch presses
        return;
    }
    if (sw > NUM_SWITCHES) return;
    // loop through all the sections to work out with which section the switch
    // is associated.
    section=switch2Section[sw];
    if (section >= NUM_SECTIONS) return;
    if (sw == sections[section].request_switch) {
        if (isOurControlled(section)) {
            releaseControl(section);
        } else {
            if (isOtherControlled(section)) {
                if (NV->flags & NV_FLAG_MASTER_PANEL) {
                    requestControl(section);
                }
            } else {
                requestControl(section);
            }
        }
        return;
    }
}

void requestControl(unsigned char section) {
    // Tell other panels we are taking control  
    unsigned char nnl = NV->sections[section].section_nn_bytes.section_nn_l;
    unsigned char nnh = NV->sections[section].section_nn_bytes.section_nn_h;
    cbusMsg[d5] = nnh;
    cbusMsg[d6] = nnl;
    cbusMsg[d7] = NV->sections[section].section_en_bytes.section_en_l;
    if ((nnh == 0 ) && (nnl == 0)) return;
    if (getProducedEvent(HAPPENING_SECTION_CONTROL)) {
        // This should be a ASON3
        cbusSendEventWithData( CBUS_OVER_CAN, 0, producedEvent.EN, 1, cbusMsg, 3);
    }
    
    clearLed(sections[section].otherControlled_led);
    setLed(sections[section].ourControl_led);
}

void releaseControl(unsigned char section) {
    unsigned char nnl = NV->sections[section].section_nn_bytes.section_nn_l;
    unsigned char nnh = NV->sections[section].section_nn_bytes.section_nn_h;
    if ((nnh == 0 ) && (nnl == 0)) return;
    
    clearLed(sections[section].otherControlled_led);
    clearLed(sections[section].ourControl_led);
    if (NV->flags & NV_FLAG_STOP_ON_RELEASE) {
        setSpeed(section, 0);
    }
    // Tell other panels we have released control  
    cbusMsg[d5] = nnh;
    cbusMsg[d6] = nnl;
    cbusMsg[d7] = NV->sections[section].section_en_bytes.section_en_l;
    if (getProducedEvent(HAPPENING_SECTION_CONTROL)) {
        // This should be a ASOF3
        cbusSendEventWithData( CBUS_OVER_CAN, 0, producedEvent.EN, 0, cbusMsg, 3);
    }
}

unsigned char isOurControlled(unsigned char section) {
    return testLed(sections[section].ourControl_led);
}

unsigned char isOtherControlled(unsigned char section) {
    return testLed(sections[section].otherControlled_led    );
}


void gotOtherControlledMessage(unsigned char section) {
    // we didn't send the message so another panel has control
    setLed(sections[section].otherControlled_led);
    clearLed(sections[section].ourControl_led);
}
void lostOtherControlledMessage(unsigned char section) {
    clearLed(sections[section].otherControlled_led);
    clearLed(sections[section].ourControl_led);
}

void receivedControlMessage(unsigned char * rx_ptr) {
    unsigned char section;
    unsigned char opc = rx_ptr[d0];
    
    unsigned char nnh = rx_ptr[d5];
    unsigned char nnl = rx_ptr[d6];
    unsigned char enl = rx_ptr[d7];
    unsigned char enh = 0;
    
    // look for this section in the NVs
    for (section = 0; section <NUM_SECTIONS; section++) {
        if (NV->sections[section].section_nn_bytes.section_nn_h != nnh) continue;
        if (NV->sections[section].section_nn_bytes.section_nn_l != nnl) continue;
        if (NV->sections[section].section_en_bytes.section_en_h != enh) continue;
        if (NV->sections[section].section_en_bytes.section_en_l != enl) continue;
        
        // It is for one of the sections we are managing
        if (opc&1) {
            // OFF event
            lostOtherControlledMessage(section);
        } else {
            // ON event
            gotOtherControlledMessage(section);
        }
    }
}
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
#include "sections.h"
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

const Section sections[NUM_SECTIONS] = {
    {0,  1,  0,  8},  //0
    {2,  3, 16, 24},  //1
    {4,  5,  1,  9},  //2
    {6,  7, 17, 25},  //3
    {8,  9,  2, 10},  //4
    {10,11, 18, 26}, //5
    {12,13,  3, 11}, //6
    {14,15, 19, 27}, //7
    {16,17,  4, 12}, //8
    {18,19, 20, 28}, //9
    {20,21,  5, 13}, //10
    {22,23, 21, 29}, //11
    {24,25,  6, 14}, //12
    {26,27, 22, 30}, //13
    {28,29,  7, 15}, //14
    {30,31, 23, 31}  //15 
};

void switch_pressed(unsigned char sw) {
    unsigned char section;
    
    for (section=0; section<NUM_SECTIONS; section++) {
        if (sw == sections[section].request_switch) {
            if (NV->flags & NV_FLAG_SWITCH_TOGGLE) {
                if (haveControl(section)) {
                    releaseControl(section);
                } else {
                    if (isControlled(section)) {
                        if (NV->flags & NV_FLAG_MASTER_PANEL) {
                            requestControl(section);
                        }
                    } else {
                        requestControl(section);
                    }
                }
            } else {
                if (isControlled(section)) {
                    // if someone already has control then we need Master_Panel permission
                    if (NV->flags & NV_FLAG_MASTER_PANEL) {
                        requestControl(section);
                    }
                } else {
                    requestControl(section);
                }
            }
            return;
        } else if (sw == sections[section].release_switch) {
            if (NV->flags & NV_FLAG_SWITCH_TOGGLE) {
                // in toggle mode ignore the release switch
            } else {
                releaseControl(section);
            }
            return;
        }
    }
}

void requestControl(unsigned char section) {
    // Tell other panels we are taking control  
    cbusMsg[d5] = NV->sections[section].nv_section_nn.section_nn_bytes.section_nn_h;
    cbusMsg[d6] = NV->sections[section].nv_section_nn.section_nn_bytes.section_nn_l;
    cbusMsg[d7] = NV->sections[section].nv_section_en.section_en_bytes.section_en_l;
    if (getProducedEvent(ACTION_PRODUCER_SECTION_CONTROL)) {
        // This should be a ASON3
        cbusSendEventWithData( CBUS_OVER_CAN, 0, producedEvent.EN, 1, cbusMsg, 3);
    }
    
    setLed(sections[section].controlled_led);
    setLed(sections[section].haveControl_led);
}

void releaseControl(unsigned char section) {
    clearLed(sections[section].controlled_led);
    clearLed(sections[section].haveControl_led);
    if (NV->flags & NV_FLAG_STOP_ON_RELEASE) {
        setSpeed(section, 0);
    }
    // Tell other panels we have released control  
    cbusMsg[d5] = NV->sections[section].nv_section_nn.section_nn_bytes.section_nn_h;
    cbusMsg[d6] = NV->sections[section].nv_section_nn.section_nn_bytes.section_nn_l;
    cbusMsg[d7] = NV->sections[section].nv_section_en.section_en_bytes.section_en_l;
    if (getProducedEvent(ACTION_PRODUCER_SECTION_CONTROL)) {
        // This should be a ASOF3
        cbusSendEventWithData( CBUS_OVER_CAN, 0, producedEvent.EN, 0, cbusMsg, 3);
    }
}

unsigned char haveControl(unsigned char section) {
    return testLed(sections[section].haveControl_led);
}

unsigned char isControlled(unsigned char section) {
    return testLed(sections[section].controlled_led);
}


void gotControlledMessage(unsigned char section) {
    // we didn't send the message so another panel has control
    setLed(sections[section].controlled_led);
    clearLed(sections[section].haveControl_led);
}
void lostControlledMessage(unsigned char section) {
    clearLed(sections[section].controlled_led);
    clearLed(sections[section].haveControl_led);
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
        if (NV->sections[section].nv_section_nn.section_nn_bytes.section_nn_h != nnh) continue;
        if (NV->sections[section].nv_section_nn.section_nn_bytes.section_nn_l != nnl) continue;
        if (NV->sections[section].nv_section_en.section_en_bytes.section_en_h != enh) continue;
        if (NV->sections[section].nv_section_en.section_en_bytes.section_en_l != enl) continue;
        
        // It is for one of the sections we are managing
        if (opc&1) {
            // OFF event
            lostControlledMessage(section);
        } else {
            // ON event
            gotControlledMessage(section);
        }
    }
}
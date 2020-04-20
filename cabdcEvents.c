
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
 * File:   mioEvents.c
 * Author: Ian Hogg
 * 
 * Here we deal with the module specific event handling. This covers:
 * <UL>
 * <LI>Setting of default events.</LI>
 * <li>Processing of inbound, consumed events</LI>
 *</UL>
 * 
 * Created on 10 April 2017, 10:26
 */

#include <stddef.h>
#include "module.h"
#include "GenericTypeDefs.h"
#include "cabdcEvents.h"
#include "cbus.h"
#include "FliM.h"
#include "sections.h"


extern BOOL	thisNN( BYTE *rx_ptr);
// forward declarations

static TickValue startWait;

void cabdcEventsInit(void) {
    startWait.Val = 0;
}

/**
 * Set Global Events back to factory defaults.
 */
void factoryResetGlobalEvents(void) {
    // we don't create a default SOD event
    //      NN,  EN,                EV#, EVval,                  , force Own NN
    // A short event with default EN
    addEvent(0, DEFAULT_SECTION_EN, 0, HAPPENING_SECTION_CONTROL, FALSE);
}


/**
 * This is called by the CBUS library to see if any software generated events are to be sent.
 * Just return false as we no longer want to support this.
 * 
 * @param paction
 * @return false
 */
BOOL getDefaultProducedEvent(HAPPENING_T happening) {
    return FALSE;
}


/**
 * Process the consumed events. 
 * 
 * The only events we should receive are the panel gotControl and lostControl messages.
 * 
 * @param tableIndex the required action to be performed.
 * @param msg the full CBUS message so that OPC  and DATA can be retrieved.
 */
void processEvent(BYTE tableIndex, BYTE * msg) {
   // check if the nn is me and therefore if this is a self-consumed event
    
    if (thisNN(msg)) {
        // unusually we don't want to handle our own events
        return;
    }
    receivedControlMessage(msg);
}



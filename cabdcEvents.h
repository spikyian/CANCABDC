
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
 * File:   mioEvents.h
 * Author: Ian
 *
 * Created on 17 April 2017, 13:14
 */

#ifndef MIOEVENTS_H
#define	MIOEVENTS_H

#ifdef	__cplusplus
extern "C" {
#endif
    /*
     * This is where all the module specific actions are defined.
     * The following definitions are required by the FLiM code:
     * NUM_PRODUCER_ACTIONS, NUM_CONSUMER_ACTIONS, HASH_LENGTH, EVT_NUM, 
     * EVperEVT, NUM_CONSUMED_EVENTS
     * 
     * For MIO an action is a BYTE (unsigned char). The upperupper bit is used
     * to indicate a sequential action. It would be nice to define:
     * 
     * typedef union {
     *      unsigned char action_byte;
     *      struct {
     *          unsigned char action :7;
     *          unsigned char sequential :1;
     *      }
     * } ACtION_T;
     * but C spec doesn't define what size this would be. Therefore I just use
     * BYTE (unsigned char).
     * 
     * The actions are defined below - but remember consumed actions may also have MSB
     * set indicating sequential.
     */
 
#include "candccab.h"

#define NUM_PRODUCER_ACTIONS    2
#define ACTION_PRODUCER_BASE    1
    
#define DEFAULT_SECTION_EN  0xEA01
/* CONSUMED actions */
    // Global consumed actions first

    
/* PRODUCED actions */    
    // Global produced actions next
#define ACTION_PRODUCER_SECTION_CONTROL     2   // used for both the consumed and produced event
#define ACTION_PRODUCER_SOD                 1

extern void mioEventsInit(void);
extern void factoryResetGlobalEvents(void);
extern void defaultEvents(unsigned char i, unsigned char type);
extern void clearEvents(unsigned char i);

// These are chosen so we don't use too much memory 32*20 = 640 bytes.
// Used to size the hash table used to lookup events in the events2actions table.
#define HASH_LENGTH     2
#define CHAIN_LENGTH    2

#define NUM_EVENTS              2           // must be less than 256 otherwise loops fail
#define EVENT_TABLE_WIDTH       1          // Width of eventTable
#define EVperEVT                1          // Max number of EVs per event
#ifdef __18F25K80
#define AT_EVENTS               0x6F80      //(AT_NV - sizeof(EventTable)*NUM_EVENTS) Size=256 * 22 = 5632(0x1600) bytes
#endif
#ifdef __18F26K80
#define AT_EVENTS               0xEF80      //(AT_NV - sizeof(EventTable)*NUM_EVENTS) Size=256 * 22 = 5632(0x1600) bytes
#endif

// We'll also be using configurable produced events
#define PRODUCED_EVENTS
#define ConsumedActionType  BYTE;

extern void processEvent(BYTE eventIndex, BYTE* message);
extern void processActions(void);

#include "events.h"

extern BOOL sendInvertedProducedEvent(PRODUCER_ACTION_T action, BOOL state, BOOL invert);

#ifdef	__cplusplus
}
#endif

#endif	/* MIOEVENTS_H */


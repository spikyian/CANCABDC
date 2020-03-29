
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
 * File:   main.c
 * Author: Ian Hogg
 * 
 * This is the main for the Configurable CANMIO module.
 * 
 * Timer usage:
 * TMR0 used in ticktime for symbol times. Used to trigger next set of servo pulses
 * TMR1 Servo outputs 0, 4, 8, 12
 * TMR2 Servo outputs 1, 5, 9, 13
 * TMR3 Servo outputs 2, 6, 10, 14
 * TMR4 Servo outputs 3, 7, 11, 15
 *
 * Created on 10 April 2017, 10:26
 */
/** TODOs

 * Work out what to do if all CANIDs are taken can18.c
 * Check handling of REQEV events.c
 * Consider option to set outputs on or off on powerup in addition to restore previous state
 * Heartbeat message
 * Randomise bounce
 * RFID input
 * CHECK DOCS
 *
 * 
 * DONES:
 * DONE  Flicker LED on CAN activity 
 * DONE  SERVO/Bounce/MULT invert action
 * DONE  two on produced event
 * DONE  Implement ENUM    force of self enumeration
 * DONE  Implement CANID   set canId
 * DONE  Check handling of REVAL events.c
 * DONE  Implement NNRST
 * DONE  Implement NNRSM
 * DONE  needsStarting in Pulse OUTPUT
 * DONE  Change order of Pin Configs 0-7 done. 8-15 need checking
 * DONE  Implement AREQ but doesn't handle default events
 * DONE  Fix saved events when doing SNN - not needed
 * DONE  Flash OUTPUT type
 * DONE  Implement NENRD
 * DONE  Extend ActionQueue size
 * DONE  Determine how to send lots of CBUS messages without filling TX buffers
 * DONE  Consider a delay action for sequences
 * DONE  Check handling of NERD is correct and produces correct ENRSP events.c
 * DONE  change the START_SOD_EVENT for a learned action/event
 * DONE  consumed event processing
 * DONE  validate NV changes
 * DONE  servo outputs
 * DONE  debounce inputs
 * DONE  invert inputs
 * DONE  invert outputs
 * DONE  multi-position outputs
 * DONE  Fix deleteAction events.c
 * DONE  Pulse outputs
 * DONE  Bounce algorithm for servos
 * DONE  Store output state in EEPROM + restore on powerup
 * DONE  add a max loop count for bounce calculations
 * DONE  Change doAction to properly check for global actions
 * DONE  Move SOD processing to doAction from processEvent
 * DONE  Fix the SOD processing to include mid events
 * DONE  remember output state in EEPROM outputs.c & servo.c
 * DONE  Need more config changes when changing type
 * DONE  NV change callback for type change
 * DONE  add needsStarting and completed for OUTPUT types so can be processed sequentially
 * DONE  sequence servos servo.c
 * DONE  Bootloader and handling of OPC_BOOT
 * DONE  Fix INVERTED for all types
 * DONE  Analogue inputs for magnetic and current sense detectors
 * 
 * 
 * FCU changes needed:
 * * The number of event slots used + number of free slots != total number of events
 * * Support for CMDERR(NO_EV) when doing REVAL
 * * Setting of one NV (type) can effect other NVs. Should read back all NVs after setting one
 * * Variable number of EVs per event up to the maximum
 * * Event action sequences
 * * A module can consume its own events
 * * A NN can be upto 65535 (suggestion)
 * * Depending upon decision on default events changes may be required
 */

/**
 *	The Main CANMIO program supporting configurable I/O.
 */

#include "devincs.h"
#include <stddef.h>
#include "module.h"
#include "candccab.h"
#include "cabdcFLiM.h"
#include "StatusLeds.h"
#include "cabdcEEPROM.h"
#include "events.h"
#include "cabdcNv.h"
#include "FliM.h"
#include "romops.h"
#include "can18.h"
#include "cbus.h"

#include "analogue.h"
#include "potentiometer.h"
#include "switches.h"
#include "leds.h"
#include "tests.h"

#ifdef NV_CACHE
#include "nvCache.h"
#endif

#ifdef __18CXX
void ISRLow(void);
void ISRHigh(void);
#endif


// forward declarations
void __init(void);
BOOL checkCBUS( void);
void ISRHigh(void);
void initialise(void);
void factoryReset(void);
void factoryResetGlobalNv(void);
BOOL sendProducedEvent(unsigned char action, BOOL on);
void factoryResetEE(void);
void factoryResetFlash(void);


#ifdef __18CXX
void high_irq_errata_fix(void);

/*
 * Interrupt vectors (moved higher when bootloader present)
 */

// High priority interrupt vector

#ifdef BOOTLOADER_PRESENT
    #pragma code high_vector=0x808
#else
    #pragma code high_vector=0x08
#endif


//void interrupt_at_high_vector(void)

void HIGH_INT_VECT(void)
{
    _asm
        CALL high_irq_errata_fix, 1
    _endasm
}

/*
 * See 18F2480 errata
 */
void high_irq_errata_fix(void) {
    _asm
        POP
        GOTO ISRHigh
    _endasm
}

// low priority interrupt vector

#ifdef BOOTLOADER_PRESENT
    #pragma code low_vector=0x818
#else
    #pragma code low_vector=0x18
#endif

void LOW_INT_VECT(void)
{
    _asm GOTO ISRLow _endasm
}
#endif

TickValue   lastSwitchPollTime;
TickValue   lastLedPollTime;
TickValue   lastAnaloguePollTime;
static TickValue   lastPotentiometerPollTime;
TickValue   startTime;
static BOOL        started = FALSE;
#define ANALOGUE_PORT 4

#ifdef BOOTLOADER_PRESENT
// ensure that the bootflag is zeroed
#pragma romdata BOOTFLAG
rom BYTE eeBootFlag = 0;
#endif

// MAIN APPLICATION
#pragma code
/**
 * It is all run from here.
 * Initialise everything and then loop receiving and processing CAN messages.
 */
#ifdef __18CXX
void main(void) {
#else
int main(void) @0x800 {
#endif
    unsigned char rcon = RCON;
    initRomOps();
#ifdef NV_CACHE
    // If we are using the cache make sure we get the NVs early in initialisation
    NV = loadNvCache(); // replace pointer with the cache
#endif
    // Both LEDs off to start with during initialisation
    initStatusLeds();

    startTime.Val = tickGet();
    lastSwitchPollTime.Val = startTime.Val;
    lastLedPollTime.Val = startTime.Val;
    lastAnaloguePollTime.Val = startTime.Val;
    lastPotentiometerPollTime.Val = startTime.Val;
  
    // check if PB is held down during power up
    if (!FLiM_SW) {
        // test mode

        initTicker(0);  // set low priority
        // Disable PORT B weak pullups
        INTCON2 = 0;
        // Disable weak pullups
        WPUB = 0;

        // set the servo state and positions before configuring IO so we reduce the startup twitch
        initAnalogue(ANALOGUE_PORT);
        initPotentiometer();
        initSwitches();
        initLeds();
        pollSwitches(0); // read the first column of 4 switches 
        test2();
        if (switch_matrix[0]&0x02) {    // second button pressed
            test2();
        }
        if (switch_matrix[0]&0x04) {    // third button pressed
            test3();
        }
        test1();
    }
    
    initialise(); 

    while (TRUE) {
        // Startup delay for CBUS about 2 seconds to let other modules get powered up - ISR will be running so incoming packets processed
        if (!started && (tickTimeSince(startTime) > (NV->sendSodDelay * HUNDRED_MILI_SECOND) + TWO_SECOND)) {
            started = TRUE;
            if (NV->sendSodDelay > 0) {
                sendProducedEvent(ACTION_PRODUCER_SOD, TRUE);
            }
        }
        checkCBUS();    // Consume any CBUS message and act upon it
        FLiMSWCheck();  // Check FLiM switch for any mode changes
        
        if (started) {
            if (tickTimeSince(lastAnaloguePollTime) > (6 * ONE_MILI_SECOND)) {
                pollAnalogue(ANALOGUE_PORT);
                lastAnaloguePollTime.Val = tickGet();
            }
            if (tickTimeSince(lastPotentiometerPollTime) > (19 * ONE_MILI_SECOND)) {
                pollPotentiometer();
                lastPotentiometerPollTime.Val = tickGet();
            }
            if (tickTimeSince(lastSwitchPollTime) > (2 * ONE_MILI_SECOND)) {
                pollSwitches(1);
                lastSwitchPollTime.Val = tickGet();
            }
            if (tickTimeSince(lastLedPollTime) > (2 * ONE_MILI_SECOND)) {
                pollLeds();
                lastLedPollTime.Val = tickGet();
            }
        }
     } // main loop
} // main
 

/**
 * The order of initialisation is important.
 */
void initialise(void) {
    // don't enable the 4x PLL. Will be done in CONFIG as using 4MHz clock
    //OSCTUNEbits.PLLEN = 1; 
    
    // check if EEPROM is valid
   if (ee_read((WORD)EE_VERSION) != EEPROM_VERSION) {
        // may need to upgrade of data in the future
        // set EEPROM to default values
        factoryResetEE();
        // If the FCU has requested EE rewrite then they also want to reset events and NVs
        factoryResetFlash();
        // set the reset flag to indicate it has been initialised
        ee_write((WORD)EE_VERSION, EEPROM_VERSION);
        writeFlashByte((BYTE*)(AT_NV + NV_VERSION), (BYTE)FLASH_VERSION);
#ifdef NV_CACHE
        loadNvCache();                
#endif
    }
    // check if FLASH is valid
    if (NV->nv_version != FLASH_VERSION) {
        // set Flash to default values
        factoryResetFlash();
        
         // set the version number to indicate it has been initialised
        writeFlashByte((BYTE*)(AT_NV + NV_VERSION), (BYTE)FLASH_VERSION);
#ifdef NV_CACHE
        loadNvCache();                
#endif
    }
    initTicker(0);  // set low priority
    // Disable PORT B weak pullups
    INTCON2 = 0;
    // Disable weak pullups
    WPUB = 0;

    cabdcEventsInit();
    cabdcFlimInit(); // This will call FLiMinit, which, in turn, calls eventsInit, cbusInit
    
    // set the servo state and positions before configuring IO so we reduce the startup twitch
    initAnalogue(ANALOGUE_PORT);
    initPotentiometer();
    initSwitches();
    initLeds();

    /*
     * Now configure the interrupts.
     * Interrupt priority is enabled with the High priority interrupt used for
     * the servo timers and Low priority interrupt used for CAN and tick timer.
     */
    
    // Enable interrupt priority
    RCONbits.IPEN = 1;
    // enable interrupts, all init now done
    ei(); 
}

/**
 * set EEPROM to default values
 */
void factoryResetEE(void) {
    ee_write((WORD)EE_BOOT_FLAG, 0);
    ee_write((WORD)EE_CAN_ID, DEFAULT_CANID);
    ee_write_short((WORD)EE_NODE_ID, DEFAULT_NN); 
    ee_write((WORD)EE_FLIM_MODE, fsSLiM);
}

/**
 * Set up the EEPROM and Flash.
 * Should get called once on first power up or upon NNRST CBUS command. 
 * Initialise EEPROM and Flash.
 */
void factoryReset(void) {
    factoryResetEE();
    factoryResetFlash();
}

void factoryResetFlash(void) {
    factoryResetGlobalNv();
    clearAllEvents();
    factoryResetGlobalEvents();
    flushFlashImage();
}

/**
 * Check to see if now is a good time to start a flash write.
 * @return 
 */
unsigned char isSuitableTimeToWriteFlash() {
    return TRUE;    // we generally don't have anything better to do
}


/**
 * Check to see if a message has been received on the CBUS and process 
 * it if one has been received.
 * @return true if a message has been received.
 */
BOOL checkCBUS( void ) {
    BYTE    msg[20];

    if (cbusMsgReceived( 0, (BYTE *)msg )) {
        shortFlicker();         // short flicker LED when a CBUS message is seen on the bus
        if (parseCBUSMsg(msg)) {               // Process the incoming message
            longFlicker();      // extend the flicker if we processed the message
            return TRUE;
        }
        if (thisNN(msg)) {
            // handle the CANMIO specifics
            switch (msg[d0]) {
            case OPC_NNRSM: // reset to manufacturer defaults
                if (flimState == fsFLiMLearn) {
                    factoryReset();
                }
                else 
                {
                    cbusMsg[d3] = CMDERR_NOT_LRN;
                    cbusSendOpcMyNN( 0, OPC_CMDERR, cbusMsg);
                }
                return TRUE;
            case OPC_NNRST: // restart
                // if we just call main then the stack won't be reset and we'd also want variables to be nullified
                // instead call the RESET vector (0x0000)
                Reset();
            }
        }
    }
    return FALSE;
}


#ifdef __18CXX
// C intialisation - declare a copy here so the library version is not used as it may link down in bootloader area
extern const rom near BYTE * NvBytePtr;

extern rom near EventTable * eventTable;

void __init(void)
{
    // if using c018.c the routine to initialise data isn't called. Explicitly setting here is more efficient
    NvBytePtr = (const rom near BYTE*)AT_NV;
    eventTable = (rom near EventTable*)AT_EVENTS;
}

// Interrupt service routines
#if defined(__18CXX)
#pragma interruptlow ISRLow
void ISRLow(void) {
#elif defined(__dsPIC30F__) || defined(__dsPIC33F__) || defined(__PIC24F__) || defined(__PIC24FK__) || defined(__PIC24H__)
    void _ISRFAST __attribute__((interrupt, auto_psv)) _INT1Interrupt(void)
#elif defined(__PIC32MX__)
    void __ISR(_EXTERNAL_1_VECTOR, ipl4) _INT1Interrupt(void)
#else
    void interrupt low_priority low_isr(void) {
#endif
    tickISR();
    canInterruptHandler();
}

// Interrupt service routines

#if defined(__18CXX)
#pragma interruptlow ISRHigh
void ISRHigh(void) {
#elif defined(__dsPIC30F__) || defined(__dsPIC33F__) || defined(__PIC24F__) || defined(__PIC24FK__) || defined(__PIC24H__)
    void _ISRFAST __attribute__((interrupt, auto_psv)) _INT1Interrupt(void)
#elif defined(__PIC32MX__)
    void __ISR(_EXTERNAL_1_VECTOR, ipl4) _INT1Interrupt(void)
#else 
    void interrupt high_priority high_isr (void) {
#endif

}



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
 * This is the main for the CANCABDC module.
 * 
 * Timer usage:
 * TMR0 used in ticktime for symbol times. Used to trigger next set of servo pulses
 *
 * Created on 10 March 2020, 10:26
 */
/** TODOs
 * Work out what to do if all CANIDs are taken can18.c
 * Change the switches for the test routines
 * Add proper Module Id
 * Add COE condition into library
 * Allow ASON/ASOF to control our controls
 */

/**
 *	The Main CANCABDC
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
#include "sections.h"
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
static TickValue   lastSyncTime;
TickValue   startTime;
static BOOL started;
static BOOL canInitialised;

#define ANALOGUE_PORT 4

#ifdef BOOTLOADER_PRESENT
// ensure that the bootflag is zeroed
#pragma romdata BOOTFLAG
rom BYTE eeBootFlag = 0;
#endif

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
#define TEST_REPEAT     24 

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
    unsigned char i;
    canInitialised = FALSE;
    initRomOps(); 
#ifdef NV_CACHE
    // If we are using the cache make sure we get the NVs early in initialisation
    NV = loadNvCache(); // replace pointer with the cache
#endif
    // enable the 4x PLL. 
    OSCTUNEbits.PLLEN = 1; 
    /*
     * Now configure the interrupts.
     * Interrupt priority is enabled with the Low priority interrupt used for CAN and tick timer.
     */
    // Enable interrupt priority, clear CM, clear RESET, clear WDT, clear TO, clear POR, clear BOR
    RCON =0xBF;
    
    // Both LEDs off to start with during initialisation
    initStatusLeds();
    TRISA=0x08;     // set up PB as input
    initAnalogue(ANALOGUE_PORT);
    
    // check if PB is held down during power up
    if ( ! FLiM_SW) 
    {
        initTicker(0);  // set low priority
        // Disable PORT B weak pullups
        INTCON2bits.RBPU = 1;
        // Disable weak pullups
        WPUB = 0;

        initPotentiometer();
        initSwitches();
        initLeds();

        // all init now done, enable interrupts
        ei();
        startTime.Val = tickGet();

        for (i=0; i<8; i++) {
            pollSwitches(0); // read col  switches
        }
        
        startFLiMFlash(TRUE);   // fast flashing to indicate self test
        
        if (switch_matrix[1]) {    // first switch on or first button
            test2();
        }
        if (switch_matrix[2]) {    // second switch on
            test3();
        }
        test1();
    }
   
    initialise(); 
 
    started = FALSE;
    
    startTime.Val = tickGet();
    lastSwitchPollTime.Val = startTime.Val;
    lastLedPollTime.Val = startTime.Val;
    lastAnaloguePollTime.Val = startTime.Val;
    lastPotentiometerPollTime.Val = startTime.Val;
    lastSyncTime.Val = startTime.Val;

    while (TRUE) {
        // Startup delay for CBUS about 2 seconds to let other modules get powered up - ISR will be running so incoming packets processed
        if (!started && (tickTimeSince(startTime) > (NV->sendSodDelay * HUNDRED_MILI_SECOND) + TWO_SECOND)) {
            started = TRUE;
            if (NV->sendSodDelay > 0) {
                sendProducedEvent(HAPPENING_SOD, TRUE);
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
            if ((NV->sync_tx > 0) && (tickTimeSince(lastSyncTime) > (100 * ONE_MILI_SECOND * NV->sync_tx))) {
                cbusMsg[d0] = OPC_TON;
                cbusSendMsg(ALL_CBUS, cbusMsg);     // send a sync 
                lastSyncTime.Val = tickGet();
            }
        }
        // Check for any flashing status LEDs
        checkFlashing();
     } // main loop
} // main


/**
 * The order of initialisation is important.
 */
void initialise(void) {
    
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
    INTCON2bits.RBPU = 1;
    // Disable weak pullups
    WPUB = 0;

    cabdcEventsInit();
    cabdcFlimInit(); // This will call FLiMinit, which, in turn, calls eventsInit, cbusInit
    canInitialised = TRUE;
    
    // set the servo state and positions before configuring IO so we reduce the startup twitch
    initPotentiometer();
    initSwitches();
    initLeds();
    initSections();

    
    // all init now done, enable interrupts
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
    if (canInitialised) {
        canInterruptHandler();
    }
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


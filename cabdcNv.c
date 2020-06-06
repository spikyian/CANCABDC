
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
 * File:   nodeVariables.c
 * Author: Ian Hogg
 *
 * Created on 05 March 2016, 18:14
 */

/**
 * Node variables contain global parameters that need to be persisted to Flash.
 */

#include "devincs.h"
#include "module.h"
#include "cabdcNv.h"
#include "cabdcEEPROM.h"
#include "events.h"
#include "romops.h"
#include "FliM.h"
#ifdef NV_CACHE
#include "nvCache.h"
#endif
#include "cbus.h"
#include "analogue.h"

#ifdef __XC8
const ModuleNvDefs moduleNvDefs @AT_NV; // = {    //  Allow 128 bytes for NVs. Declared const so it gets put into Flash
#else
//#pragma romdata myNV=AT_NV
#endif
/*
 Module specific NV routines
 */
#ifdef __XC8
const NodeVarTable nodeVarTable @AT_NV;
ModuleNvDefs * NV = (ModuleNvDefs*)&(moduleNvDefs);    // pointer to the NV structure
#else
#ifdef NV_CACHE
ModuleNvDefs * NV; // = &(nodeVarTable.moduleNVs);
#else
volatile rom near ModuleNvDefs * NV = (volatile rom near ModuleNvDefs*)&(nodeVarTable.moduleNVs);    // pointer to the NV structure
#endif
#endif

void cabdcNvInit(void) {

}

/**
 * Validate value of NV based upon bounds and inter-dependencies.
 * @return TRUE is a valid change
 */
BOOL validateNV(unsigned char index, unsigned char oldValue, unsigned char value) {
    switch (index) {
        case NV_POT_DEAD_ZONE:
        case NV_POT_START_LEVEL:
        case NV_POT_END_LEVEL:
        case NV_ACCELERATION:
            // These must be 0-127
            if (value & 0x80) {
                return FALSE;
            }
            break;
    }
    return TRUE;
} 

void actUponNVchange(unsigned char index, unsigned char oldValue, unsigned char value) {

}


/**
 * Set NVs back to factory defaults.
 */
void factoryResetGlobalNv(void) {
    unsigned char i;
    
    writeFlashByte((BYTE*)(AT_NV + NV_SOD_DELAY), (BYTE)0);
    writeFlashByte((BYTE*)(AT_NV + NV_POT_DEAD_ZONE), (BYTE)10);
    writeFlashByte((BYTE*)(AT_NV + NV_POT_START_LEVEL), (BYTE)5);
    writeFlashByte((BYTE*)(AT_NV + NV_POT_END_LEVEL), (BYTE)127);
    writeFlashByte((BYTE*)(AT_NV + NV_ACCELERATION), (BYTE)1);
    writeFlashByte((BYTE*)(AT_NV + NV_FREQUENCY), (BYTE)1);
    writeFlashByte((BYTE*)(AT_NV + NV_FLAGS), (BYTE)(NV_FLAG_MASTER_PANEL | NV_FLAG_STOP_ON_RELEASE ));
    writeFlashByte((BYTE*)(AT_NV + NV_SYNC_TX), (BYTE)0);
    
    // Now reset the per section NVs
    for (i=0; i< NUM_SECTIONS; i++) {
        writeFlashByte((BYTE*)(AT_NV + NV_SECTION_NN_H(i)), (BYTE)0);
        writeFlashByte((BYTE*)(AT_NV + NV_SECTION_NN_L(i)), (BYTE)0);
        writeFlashByte((BYTE*)(AT_NV + NV_SECTION_EN_H(i)), (BYTE)0);
        writeFlashByte((BYTE*)(AT_NV + NV_SECTION_EN_L(i)), (BYTE)(i%4));   // CAN4DC uses EN 0-3
    }

#ifdef NV_CACHE
    loadNvCache();
#endif
}



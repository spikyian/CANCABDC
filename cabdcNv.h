
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
 * File:   nodeVarables.h
 * Author: 	Ian Hogg
 * Comments:	Definitions for NVs
 * Revision history: 
 */

#ifndef XC_NODE_VARIABLES_H
#define	XC_NODE_VARIABLES_H

/**
 * This is where the module specific NVs are specified.
 * NVs can be accessed either by byte offset (for read/set of NVs in FLiM.c)
 * or by use of a structure with named elements. These two must be kept synchronised.
 * 
 * The following are required by FLiM.c: NV_NUM, AT_NV
 */

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */
    
#include "GenericTypeDefs.h"
#include "candccab.h"

#define FLASH_VERSION   0x02        // Version 2 introduced when Actions were changed to have 5 per channel
    
// Global NVs
#define NV_VERSION                      0
#define NV_SOD_DELAY                    1
#define NV_POT_DEAD_ZONE                2  
#define NV_POT_START_LEVEL              3 
#define NV_POT_END_LEVEL                4
#define NV_ACCELERATION                 5
#define NV_FREQUENCY                    6
#define NV_FLAGS                        7
#define NV_SPARE1                       8
#define NV_SPARE2                       9
#define NV_SPARE3                       10
#define NV_SPARE4                       11
#define NV_SPARE5                       12
#define NV_SPARE6                       13
#define NV_SPARE7                       14
#define NV_SPARE8                       15
#define NV_SECTION_START                16
#define NVS_PER_SECTION                  4
    
// NVs per SECTION
#define NV_SECTION_NN_H_OFFSET          0
#define NV_SECTION_NN_L_OFFSET          1
#define NV_SECTION_EN_H_OFFSET          2
#define NV_SECTION_EN_L_OFFSET          3   
    

#define NV_SECTION_NN_H(i)              (NV_SECTION_START + NVS_PER_SECTION*(i) + NV_SECTION_NN_H_OFFSET)
#define NV_SECTION_NN_L(i)              (NV_SECTION_START + NVS_PER_SECTION*(i) + NV_SECTION_NN_L_OFFSET)
#define NV_SECTION_EN_H(i)              (NV_SECTION_START + NVS_PER_SECTION*(i) + NV_SECTION_EN_H_OFFSET)
#define NV_SECTION_EN_L(i)              (NV_SECTION_START + NVS_PER_SECTION*(i) + NV_SECTION_EN_L_OFFSET)

#define SECTION_NV(i)                   ((unsigned char)((i-NV_IO_START)/NVS_PER_SECTION))
#define NV_NV(i)                        ((unsigned char)((i-NV_IO_START)%NVS_PER_SECTION))
  
// Flags
#define NV_FLAG_MASTER_PANEL        1   // if set then we can forceably take control
#define NV_FLAG_STOP_ON_RELEASE     2   // if set then send a stop when releasing
#define NV_FLAG_SWITCH_TOGGLE       4   // if set then only one switch per section
    

typedef struct {
    union {
        struct {
            unsigned char section_nn_h;
            unsigned char section_nn_l;
        } section_nn_bytes;
        unsigned int section_nn;
    } nv_section_nn;
    union {
        struct {
            unsigned char section_en_h;
            unsigned char section_en_l;
        } section_en_bytes;
        unsigned int section_en;
    } nv_section_en;
} NvSection;

/*
 * This structure is required by FLiM.h
 */
typedef struct {
        BYTE nv_version;                // 0 version of NV structure
        BYTE sendSodDelay;              // 1 Time after start in 100mS (plus 2 seconds) to send an automatic SoD. Set to zero for no auto SoD
        BYTE pot_dead_zone;             // 2
        BYTE pot_start_level;           // 3
        BYTE pot_end_level;             // 4
        BYTE acceleration;              // 5
        BYTE frequency;                 // 6
        BYTE flags;                     // 7 flags
        BYTE spare[8];
        NvSection sections[NUM_SECTIONS];                 // config for each IO
} ModuleNvDefs;

#define NV_NUM  sizeof(ModuleNvDefs)     // Number of node variables
#ifdef __18F25K80
#define AT_NV   0x7F80                  // Where the NVs are stored. (_ROMSIZE - 128)  Size=128 bytes
#endif
#ifdef __18F26K80
#define AT_NV   0xFF80                  // Where the NVs are stored. (_ROMSIZE - 128)  Size=128 bytes
#endif

extern void mioNvInit(void);
extern unsigned int getNodeVar(unsigned int index);
extern void setNodeVar(unsigned int index, unsigned int value);
extern BOOL validateNV(BYTE nvIndex, BYTE oldValue, BYTE value);
void actUponNVchange(unsigned char index, unsigned char oldValue, unsigned char value);
extern void defaultNVs(unsigned char i, unsigned char type);        


#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_NODE_VARAIABLES_H */


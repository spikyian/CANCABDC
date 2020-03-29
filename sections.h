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
 * File:   sections.h
 * Author: Ian
 *
 * Created on 07 March 2020, 17:20
 */

#ifndef SECTIONS_H
#define	SECTIONS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "candccab.h"
    
    
struct Section {
    unsigned char request_switch;
    unsigned char release_switch;
    unsigned char haveControl_led;
    unsigned char controlled_led;
};
typedef struct Section Section;

extern const Section sections[NUM_SECTIONS];

extern void switch_pressed(unsigned char sw, unsigned char state);
extern void requestControl(unsigned char section);
extern void releaseControl(unsigned char section);
extern void receivedControlMessage(unsigned char * rx_ptr);
extern unsigned char haveControl(unsigned char section);
extern unsigned char isControlled(unsigned char section);

#ifdef	__cplusplus
}
#endif

#endif	/* SECTIONS_H */


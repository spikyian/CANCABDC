/* 
 * File:   switches.h
 * Author: Ian
 *
 * Created on 08 March 2020, 08:51
 */

#ifndef SWITCHES_H
#define	SWITCHES_H

#ifdef	__cplusplus
extern "C" {
#endif

    #define NUM_SWITCHES 32
    extern void initSwitches(void);
    extern void pollSwitches(unsigned char callback);
    
    extern unsigned char switch_matrix[8];

#ifdef	__cplusplus
}
#endif

#endif	/* SWITCHES_H */


/* 
 * File:   analogue.h
 * Author: Ian
 *
 * Created on 17 August 2017, 06:58
 */

#ifndef ANALOGUE_H
#define	ANALOGUE_H

#ifdef	__cplusplus
extern "C" {
#endif

extern void initAnalogue(unsigned char port);
extern void pollAnalogue(unsigned char port);

extern WORD lastReading;


#ifdef	__cplusplus
}
#endif

#endif	/* ANALOGUE_H */


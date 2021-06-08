/**
 * avr.h
 * Copyright (C) 2001-2021, Tony Givargis and Yizhen Liu
 */

#ifndef _AVR_H_
#define _AVR_H_

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/io.h>

#define XTAL_FRQ 8000000lu

#define SET_BIT(p,i) ((p) |=  (1 << (i)))
#define CLR_BIT(p,i) ((p) &= ~(1 << (i)))
#define GET_BIT(p,i) ((p) &   (1 << (i)))

#define NOP() asm volatile("nop"::)

void avr_wait(unsigned int msec);

void avr_wait2(unsigned int msec);

#endif /* _AVR_H_ */

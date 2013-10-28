/*
 * Wodox reimplemented again.
 * This code copyright (c) Luis Javier Sanz 2009-2013
 */

#include "object.h"

/*
 * These rotate objects whose orientation matters.
 */
uint8_t
rotate_object_lf(uint8_t p)
{
    switch (p) {
    case BELTLF:
	return BELTFT;
    case BELTBK:
	return BELTLF;
    case BELTRT:
	return BELTBK;
    case BELTFT:
	return BELTRT;
    }
    return p;
}

uint8_t
rotate_object_rt(uint8_t p)
{
    switch (p) {
    case BELTLF:
	return BELTBK;
    case BELTBK:
	return BELTRT;
    case BELTRT:
	return BELTFT;
    case BELTFT:
	return BELTLF;
    }
    return p;
}


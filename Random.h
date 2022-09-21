#ifndef RANDOM_H
#define RANDOM_H

#include "Random.h"
#include "common/header.h"


typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int int16_t;
typedef unsigned short int uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;

unsigned long generateSeed();
unsigned int rand_upto(unsigned int);

#endif
#include "Random.h"

unsigned long generateSeed() {
    unsigned long seed;
    uint8_t* buf = (uint8_t*) &seed;
    int n = sizeof(unsigned long);
    while(n--) {
        buf[n] = random() & 0xff;
    }
    std::cout << "seed is " << seed << std::endl;
    return seed;
}

unsigned int rand_upto(unsigned int n)
{
    return (unsigned int)(1.0 * rand() / RAND_MAX * n);
}
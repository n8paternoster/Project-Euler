#include "pch.h"
#include "51-75/63.h"

/*
    # digits of x   = floor(log10(x)) + 1
    # digits of a^b = floor(b*log10(a)) + 1
    For a >= 10:
    # digits of 10^b >= floor(b*log10(10)) + 1
                     >= b + 1
    Therefore no bases >= 10 need to be checked
*/
unsigned numNDigitNPowers() {
    unsigned count = 0;
    unsigned minBase = 1, maxBase = 10;
    for (unsigned exp = 1; minBase < maxBase; exp++) {
        while ((unsigned)(exp*log10(minBase))+1 < exp) minBase++;
        for (unsigned base = minBase; base < maxBase; base++)
            count += ((unsigned)(exp*log10(base))+1 == exp);
    }
    return count;
}

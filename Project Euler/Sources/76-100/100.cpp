#include "stdafx.h"
#include "76-100/100.h"

using ull = unsigned long long int;

/* This problem can be represented by a quadratic Diophantine equation where x = # of blue discs and y = # of red discs:

*       (x)/(x+y) * (x-1)/(x+y-1) = 1/2
*       x^2 - 2xy - y^2 - x + y = 0
* 
*  An initial solution is (x, y) = (1, 0) and recurrence equations to generate
*  all solutions, where x' = prevX and y' = prevY, are:
* 
*       x = 5*x' + 2*y' - 2
*       y = 2*x' + y' - 1
*/
ull numBlueHalfProbability(ull minTotal) {
    ull prevBlue = 1, prevRed = 0;
    ull blue = 0, red = 0;
    ull total = 0;
    while (total < minTotal) {
        blue = 5*prevBlue + 2*prevRed - 2;
        red = 2*prevBlue + prevRed - 1;
        prevBlue = blue;
        prevRed = red;
        total = blue+red;
        std::cout << blue << " blue, " << red << " red = " << total << " total\n";
    }
    return blue;
}

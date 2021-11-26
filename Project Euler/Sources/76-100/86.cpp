#include "stdafx.h"
#include "76-100/86.h"

/*
*   Let the shortest path between opposite corners be:
*
*       S = sqrt( (x-l)^2 + y^2 ) + sqrt( l^2 + z^2 )
*
*   where l is some length cut out of x along the opposite edge.
*
*   Find the value of l that minimizes S by taking the derivative of S and finding its zeros:
*
*       dS/dl = (l-x)/sqrt( (x-l)^2 + y^2 ) + l/sqrt( l^2 + z^2 ) = 0
*       l = (x*z)/(y+z)
*
*   Plug this value for l back into the shortest path formula to get:
*
*       S = sqrt( x^2 + (y+z)^2 )   when x >= y >= z
*
*   This is the shortest path when taking a path through an x-y and an x-z face. The other 2 paths involve traveling through the (y-x and y-z) faces and (z-x and z-y) faces and produce similar formulas for S. The initial formula is the shortest overall when x >= y and x >= z.
*
*   Integer solutions for S exist when the triple {x, y+z, S} is a pythagorean triple. We can generate all solutions for a given x by finding all pythagorean triples {a, b, c} where x is either a or b. For the triple {x, b, c}, the number of cuboid solutions generated is the number of ways you can partition b into y and z such that x >= y and x >= z. For the triple {a, x, c}, the number of solutions is simply the number of ways you can partition partition a into two parts (y and z) since x > a.
*/
int minMWithNCuboidSolutions(int numSolutions) {

    int x = 2;  // generate all cuboid solutions {x, y, z} where x>=y>=z, M will be x
    int count = 0;
    while (count < numSolutions) {
        x++;
        for (int p = 3; p <= 2*x; p++) {
            int q2 = x*x + p*p;
            int q = (int)std::sqrt(q2);
            if (q*q == q2 || (q+1)*(q+1) == q2) {   // pythagorean triple {x, p, q} or {p, x, q}
                count += (p>x) ? ((2*x-p)/2+1) : p/2;
            }
        }
    }
    std::cout << "m: " << x << "\ncount: " << count << "\n";
    return x;
}
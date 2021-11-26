#include "pch.h"
#include "76-100/94.h"
#include <vector>

/* An almost equilateral triangle is an isoceles triangle with sides of length s and base of length (s+1) or (s-1)
* To have an integer area its height, h, must be an integer. Using the pythagorean theorem we can write:
* 
*       h^2 + ((s+1)/2)^2 = s^2     (same process when base is s-1)
*       s^2 - (s^2 + 2s + 1)/4 - h^2 = 0
*       (3s^2 - 2s - 1)/4 - h^2 = 0
*       (9s^2 - 6s - 3)/4 - 3h^2 = 0
*       (9s^2 - 6s + 1 - 4)/4 - 3h^2 = 0
*       ((3s-1)^2 - 4)/4 - 3h^2 = 0
*       ((3s-1)/2)^2 - 3h^2 = 1
*       
*   Setting x = (3s-1)/2 and y = h we get the Pell equation:
*       
*       x^2 - 3y^2 = 1
* 
*   With an initial solution of s=5, h=4; x0=7, y0=4 and we can use recurrence relations to generate all of the other solutions:
* 
*       x{k+1} = x0*x{k} + n*y0*y{k}  ->  s{k+1} = 7*s{k} + 8*y{k} - 2
*       y{k+1} = x0*y{k} + y0*x{k}    ->  h{k+1} = 7*h{k} + 6s*{k} - 2
*/

int numAlmostEquilateralTriangles(int maxPerimeter) {
    long long totalPerimeter = 0;
    long long perimeter1 = 0, perimeter2 = 0;
    long long s1 = 1, h1 = 0;   // Larger base
    long long s2 = 1, h2 = 1;   // Smaller base
    while (perimeter1 < maxPerimeter || perimeter2 < maxPerimeter) {
        // Triangle with sides (x, x, x+1)
        long long prevS = s1;
        long long prevH = h1;
        s1 = 7*prevS + 8*prevH - 2;
        h1 = 6*prevS + 7*prevH - 2;
        perimeter1 = 2*s1 + (s1+1);
        std::cout << "Triangle (" << s1 << ", " << s1 << ", " << s1+1 << ") has area " << (s1+1)*h1/2 << "\n";
        // Triangle with sides (x, x, x-1)
        prevS = s2;
        prevH = h2;
        s2 = 7*prevS + 8*prevH + 2;
        h2 = 6*prevS + 7*prevH + 2;
        perimeter2 = 2*s2 + (s2-1);
        std::cout << "Triangle (" << s2 << ", " << s2 << ", " << s2+1 << ") has area " << (s2-1)*h2/2 << "\n";
        
        if (perimeter1 < maxPerimeter) totalPerimeter += perimeter1;
        if (perimeter2 < maxPerimeter) totalPerimeter += perimeter2;
    }
    return totalPerimeter;
}

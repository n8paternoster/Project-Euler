#include "pch.h"
#include "76-100/85.h"

int nearestGridAreaContainingRectangles(int R) {

    // The number of rectangles in a grid of n x m boxes is given by:
    // R = (n(n+1)*m(m+1))/4   

    // Find a max bound for n by setting m to 1
    // 4*R = n(n+1)
    // n ~= sqrt(8*R+1)/2 - 1/2
    int maxN = std::sqrt(8*R+1) / 2;
    int maxM = maxN;

    // Check every combination to find the one that gives a number of rectangles closest to R
    int bestNumRectangles = 0;
    int bestN = 0, bestM = 0;
    for (int n = 1; n <= maxN; n++) {
        for (int m = n; m <= maxM; m++) {
            int numRectangles = (n*(n+1)*m*(m+1))/4;
            if (std::abs(numRectangles-R) < std::abs(bestNumRectangles-R)) {
                bestNumRectangles = numRectangles;
                bestN = n;
                bestM = m;
            }
            if (numRectangles > R) break;
        }
    }
   
    std::cout << "Grid of size " << bestN << " x " << bestM << " contains " << bestNumRectangles << " rectangles\n";
    return bestN*bestM;
}
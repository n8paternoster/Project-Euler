#include "pch.h"
#include "1-25/15.h"

/*
	Draw an nxn grid and label the bottom right point 1. Label each point with the number
	of different paths you can take from that point to the bottom right (only moving down
	(and to the right) by adding the values below and to the right:
				
			   . . .
			   . . 1
 		   . . 6 3 1		 
		   . . 3 2 1
		   . 1 1 1 1		etc

	Continue numbering all points this way until reaching the top left. Drawing diagonal
	lines from the bottom left to the top right for each row corresponds to the binomial
	coefficients ((1), (1, 2, 1), (1, 3, 3, 1) etc). These rows can also be arranged into
	Pascal's triangle. The top left point of an nxn grid corresponds to the nth term on the
	(2n)th row. This value is given by:

	(2n)choose(n) = (2n)! / (n! * n!)
*/
uint64_t numLatticePathsOfNxNGrid(uint64_t n) {
	uint64_t result = 1;
	for (uint64_t num = 2*n, den = 1; num > n; num--, den++) {
		result /= den;
		result *= num;
	}
	return result;
}

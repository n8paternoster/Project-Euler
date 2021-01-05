#include "stdafx.h"
#include "15.h"
#include "bigNum.h"


//uint64_t numLatticePathsOfNxNGrid(uint64_t n) {
//	bigNum num(std::to_string(n));
//	bigNum result = factorial(num + num) / (factorial(num) * factorial(num));
//	return result;
//}

/*
	The number of lattice paths of an n by n grid is given by (n+n)! / (n! * n!).
	To visual, draw an n by n grid and label the bottom right point 0. The points that 
	border this point are labeled 1. Each of those points have only 1 possible path from
	them to the end. Now add 1 and 1 and label that result, 2, at the point that can see both
	1's. From that point there are 2 possible paths to the end. Continue numbering all points
	this way until reaching the top left most. These numbers correspond with the binomial
	coefficients and Pascal's triangle.
*/


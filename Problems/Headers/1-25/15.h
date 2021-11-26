#pragma once

/*
	Starting in the top left corner of a 2×2 grid, and only being able to move to the right and down, there are exactly 6 routes to the bottom right corner.

	[img]

	How many such routes are there through a 20×20 grid ?
*/

uint64_t numLatticePathsOfNxNGrid(uint64_t);
inline auto p15() {
	return numLatticePathsOfNxNGrid(20);
}
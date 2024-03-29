#pragma once

/*
	In the 5 by 5 matrix below, the minimal path sum from the top left to the bottom right, by only moving to the right and down, is indicated in bold red and is equal to 2427.

	[...]

	Find the minimal path sum from the top left to the bottom right by only moving right and down in matrix.txt (right click and "Save Link/Target As..."), a 31K text file containing an 80 by 80 matrix.
*/

void readMatrix(std::string file, int *data, const int N);
int minPathTwoWays(std::string, const int);
inline auto p81() {
	return minPathTwoWays("Resources/81.txt", 80);
}
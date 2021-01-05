#include "stdafx.h"
#include "18.h"
#include <fstream>
#include <sstream>

/*
By starting at the top of the triangle below and moving to adjacent numbers on the row below, the maximum total from top to bottom is 23.

	3
   7 4
  2 4 6
 8 5 9 3

That is, 3 + 7 + 4 + 9 = 23.

Find the maximum total from top to bottom of the triangle below:

			  75
			 95 64
			17 47 82
 	  	   18 35 87 10
		  20 04 82 47 65
		 19 01 23 75 03 34
		88 02 77 73 07 63 67
       99 65 04 28 06 16 70 92
      41 41 26 56 83 40 80 70 33
     41 48 72 33 47 32 37 16 94 29
    53 71 44 65 25 43 91 52 97 51 14
   70 11 33 28 77 73 17 78 39 68 17 57
  91 71 52 38 17 14 91 43 58 50 27 29 48
 63 66 04 68 89 53 67 30 73 16 69 87 40 31
04 62 98 27 23 09 70 98 73 93 38 53 60 04 23
*/

bool readData(uint64_t **data, size_t numRows) {
	// Open the file
	std::ifstream f("18.txt");
	if (!f) {
		std::cerr << "Error opening file\n";
		return false;
	}
	// Read the data: each line begins at position 0 in the array and extra spaces are zero padded
	std::string line;
	size_t row = 0;
	while (std::getline(f, line)) {
		std::istringstream iss(line);
		size_t col = 0;
		uint64_t num;
		while (iss >> num) {
			data[row][col] = num;
			col++;
		}
		row++;
	}
	return true;
}

uint64_t maxPathTotalTriangle(size_t numRows) {

	/*	Create a 2d array to represent the triangle; the 2d array is a square (numCols = numRows)
		Each row of the triangle starts at index 0 in the array and extra spaces in that row are zero padded
		A number below to the left is at index [row+1][col]
		A number below to the right is at index [row+1][col+1] */
	uint64_t **data = new uint64_t*[numRows];
	for (size_t row = 0; row < numRows; row++)
		data[row] = new uint64_t[numRows]();

	// Read the triangle data into this array
	readData(data, numRows);

	/*	Begin checking the 2nd to last row, replace each number with the sum of that number + the larger 
		child number (number directly below to the left or right) */
	for (size_t row = numRows - 2; row != -1; row--) {
		for (size_t col = 0; col <= row; col++) {
			if (data[row + 1][col] > data[row + 1][col + 1])
				data[row][col] += data[row + 1][col];
			else
				data[row][col] += data[row + 1][col + 1];
		}
	}

	uint64_t maxPathTotal = data[0][0];

	// Delete the triangle data
	for (size_t row = 0; row < numRows; row++)
		delete[] data[row];
	delete[] data;

	return maxPathTotal;
}
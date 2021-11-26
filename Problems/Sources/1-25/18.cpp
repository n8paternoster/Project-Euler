#include "pch.h"
#include "1-25/18.h"
#include <fstream>
#include <sstream>

bool readData(uint64_t **data, size_t numRows, std::string file) {
	// Open the file
	std::ifstream f(file);
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

uint64_t maxPathTotalTriangle(size_t numRows, std::string file) {

	/*	Create a 2d array to represent the triangle; the 2d array is a square (numCols = numRows)
		Each row of the triangle starts at index 0 in the array and extra spaces in that row are zero padded
		A number below to the left is at index [row+1][col]
		A number below to the right is at index [row+1][col+1] */
	uint64_t **data = new uint64_t*[numRows];
	for (size_t row = 0; row < numRows; row++)
		data[row] = new uint64_t[numRows]();

	// Read the triangle data into this array
	readData(data, numRows, file);

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
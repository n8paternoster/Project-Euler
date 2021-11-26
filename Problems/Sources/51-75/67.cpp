#include "pch.h"
#include "51-75/67.h"
#include <fstream>

int maxPathSumII(const char *file) {

	std::ifstream f(file);
	if (!f) {
		std::cerr << "Error opening file";
		return 0;
	}

	// Count the number of numbers (nodes) and make sure it is a triangular #
	int numNodes = 0;
	int n;
	while (f >> n) numNodes++;
	int root = (int)std::sqrt(8*numNodes+1);
	if (!f.eof() || root*root != 8*numNodes+1) {
		std::cerr << "File contains invalid data";
		return 0;
	}

	// Read data
	f.clear();
	f.seekg(0);
	int *nodes = new int[numNodes]();
	int i = 0;
	while (i < numNodes && f >> nodes[i]) i++;
	int numRows = ((int)std::sqrt(8*numNodes+1)-1) / 2;

	// Index the data by (row of the triangle) + (an offset from the beginning of that row)
	// Start from the second to last row and replace each node's value with its value + the greater of its two children
	for (int row = numRows-2; row >= 0; row--) {
		int rowStart = row*(row+1)/2;
		int childRowStart = rowStart + row+1;
		for (int off = 0; off <= row && childRowStart+off+1 < numNodes; off++) {
			int leftChild = nodes[childRowStart+off];
			int rightChild = nodes[childRowStart+off+1];
			nodes[rowStart+off] += leftChild > rightChild ? leftChild : rightChild;
		}
	}
	int maxSum = nodes[0];
	delete[] nodes;
	return maxSum;
}

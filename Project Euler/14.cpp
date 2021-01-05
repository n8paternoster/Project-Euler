#include "stdafx.h"
#include "14.h"

/*
The following iterative sequence is defined for the set of positive integers:

n ? n/2 (n is even)
n ? 3n + 1 (n is odd)

Using the rule above and starting with 13, we generate the following sequence:

13 ? 40 ? 20 ? 10 ? 5 ? 16 ? 8 ? 4 ? 2 ? 1
It can be seen that this sequence (starting at 13 and finishing at 1) contains 10 terms. Although it has not been proved yet (Collatz Problem), it is thought that all starting numbers finish at 1.

Which starting number, under one million, produces the longest chain?

NOTE: Once the chain starts the terms are allowed to go above one million.
*/


// Brute force solution
uint32_t longestCollatzSequenceUnderN(uint32_t n) {
	uint32_t sequenceLength;
	uint32_t maxSequenceLength = 0;
	uint32_t maxSequenceStart;
	uint32_t c;
	for (uint32_t startNum = 1; startNum < n; startNum++) {
		sequenceLength = 1;
		c = startNum;
		while (c != 1) {
			if (c % 2 == 0) {
				c = c / 2;
				sequenceLength++;
			} else {
				c = 3 * c + 1;
				sequenceLength++;
			}
		}
		if (sequenceLength > maxSequenceLength) {
			maxSequenceStart = startNum;
			maxSequenceLength = sequenceLength;
		}
	}
	std::cout << "Sequence contains " << maxSequenceLength << " terms.\n";
	return maxSequenceStart;
}
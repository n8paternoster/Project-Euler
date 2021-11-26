#include "pch.h"
#include "1-25/14.h"

// Brute force solution
uint32_t longestCollatzSequenceUnderN(uint32_t n) {
	uint32_t sequenceLength;
	uint32_t maxSequenceLength = 0;
	uint32_t maxSequenceStart = 0;
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
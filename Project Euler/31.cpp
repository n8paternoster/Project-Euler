#include "stdafx.h"
#include "31.h"

unsigned long long numCombinationsToMakeNPence(int n) {

	unsigned long long numCombinations = 0;

	for (int twoP = (n-n%200); twoP >= 0; twoP -= 200) {
		for (int p = (n-twoP)-(n-twoP)%100; p >= 0; p -= 100) {
			for (int fifty = (n-twoP-p)-(n-twoP-p)%50; fifty >= 0; fifty -= 50) {
				for (int twenty = (n-twoP-p-fifty)-(n-twoP-p-fifty)%20; twenty >= 0; twenty -= 20) {
					for (int ten = (n-twoP-p-fifty-twenty)-(n-twoP-p-fifty-twenty)%10; ten >= 0; ten -= 10) {
						for (int five = (n-twoP-p-fifty-twenty-ten)-(n-twoP-p-fifty-twenty-ten)%5; five >= 0; five -= 5) {
							for (int two = (n-twoP-p-fifty-twenty-ten-five)-(n-twoP-p-fifty-twenty-ten-five)%2; two >= 0; two -= 2) {
								numCombinations++;
							}
						}
					}
				}
			}
		}
	}

	return numCombinations;
}

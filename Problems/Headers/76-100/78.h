#pragma once

/*
	Let p(n) represent the number of different ways in which n coins can be separated into piles. For example, five coins can be separated into piles in exactly seven different ways, so p(5)=7.

	OOOOO
	OOOO   O
	OOO   OO
	OOO   O   O
	OO   OO   O
	OO   O   O   O
	O   O   O   O   O
	Find the least value of n for which p(n) is divisible by one million.
*/

int lowestPartitionDivisbleBy(int n);
inline auto p78() {
	return lowestPartitionDivisbleBy(1'000'000);
}
#include "stdafx.h"
#include "19.h"

/*
You are given the following information, but you may prefer to do some research for yourself.

1 Jan 1900 was a Monday.
Thirty days has September,
April, June and November.
All the rest have thirty - one,
Saving February alone,
Which has twenty - eight, rain or shine.
And on leap years, twenty - nine.
A leap year occurs on any year evenly divisible by 4, but not on a century unless it is divisible by 400.
How many Sundays fell on the first of the month during the twentieth century(1 Jan 1901 to 31 Dec 2000) ?
*/

int numSundaysOn1stIn20thCentury() {
	int year = 1901;
	int numSundays = 0;
	int day = 0;	// counts the number of days since 1 Jan 1901 (which is equal to 0)
	while (year < 2001) {

		// January
		if ((day + 2) % 7 == 0) numSundays++;
		day += 31;

		// February
		if ((day + 2) % 7 == 0) numSundays++;
		if (year % 4 == 0) {
			if (year % 100 == 0 && year % 400 != 0) day += 28;
			else day += 29;
		} else day += 28;

		// March
		if ((day + 2) % 7 == 0) numSundays++;
		day += 31;

		// April
		if ((day + 2) % 7 == 0) numSundays++;
		day += 30;

		// May
		if ((day + 2) % 7 == 0) numSundays++;
		day += 31;

		// June
		if ((day + 2) % 7 == 0) numSundays++;
		day += 30;

		// July
		if ((day + 2) % 7 == 0) numSundays++;
		day += 31;

		// August
		if ((day + 2) % 7 == 0) numSundays++;
		day += 31;

		// September
		if ((day + 2) % 7 == 0) numSundays++;
		day += 30;

		// October
		if ((day + 2) % 7 == 0) numSundays++;
		day += 31;

		// November
		if ((day + 2) % 7 == 0) numSundays++;
		day += 30;

		// December
		if ((day + 2) % 7 == 0) numSundays++;
		day += 31;

		year++;
	}

	return numSundays;
}
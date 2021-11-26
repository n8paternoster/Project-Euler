#include "stdafx.h"
#include "1-25/19.h"

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
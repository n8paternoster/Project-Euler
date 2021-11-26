#include "stdafx.h"
#include "76-100/84.h"
#include <string>
#include <random>		// std::mt19937, std::uniform_int_distribution, std::random_device
#include <algorithm>	// std::shuffle, std::sort
#include <deque>
#include <array>

const int numSquares = 40;
enum monopolyMoves { NOMOVE = -4, BACK3 = -3, NEXTU = -2, NEXTR = -1, GO, A1, CC1, A2, T1, R1, B1, CH1, B2, B3, JAIL, C1, U1, C2, C3, R2, D1, CC2, D2, D3, FP, E1, CH2, E2, E3, R3, F1, F2, U2, F3, G2J, G1, G2, CC3, G3, R4, CH3, H1, T2, H2 };

struct CommunityChest {
	std::deque<int> cards;
	CommunityChest(std::mt19937 gen):
		cards(std::deque<int>(14, NOMOVE))	// 14 'do nothing' cards
	{
		cards.push_back(GO);
		cards.push_back(JAIL);
		std::shuffle(cards.begin(), cards.end(), gen);
	}
	int drawCard(int currSquare) {
		int card = cards.front();
		cards.pop_front();
		cards.push_back(card);
		int newSquare = card;
		if (newSquare == NOMOVE) newSquare = currSquare;
		return newSquare;
	}
};
struct Chance {
	std::deque<int> cards;
	Chance(std::mt19937 gen):
		cards(std::deque<int>(4, NOMOVE))	// 4 'do nothing' cards
	{
		cards.push_back(GO);
		cards.push_back(JAIL);
		cards.push_back(C1);
		cards.push_back(E3);
		cards.push_back(H2);
		cards.push_back(R1);
		cards.insert(cards.end(), 2, NEXTR);
		cards.push_back(NEXTU);
		cards.push_back(BACK3);
		std::shuffle(cards.begin(), cards.end(), gen);
	}
	int drawCard(int currSquare) {
		int card = cards.front();
		cards.pop_front();
		cards.push_back(card);
		int newSquare = currSquare;
		switch (card) {
			case NOMOVE: break;
			case NEXTR:	// railroad squares are 5, 15, 25, 35
				if (currSquare >= R4 || currSquare < R1) newSquare = R1;
				else if (currSquare >= R3) newSquare = R4;
				else if (currSquare >= R2) newSquare = R3;
				else newSquare = R2;
				break;
			case NEXTU:	// utility company squares are 12, 28
				if (currSquare >= U1 && currSquare < U2) newSquare = U2;
				else newSquare = U1;
				break;
			case BACK3:
				newSquare = (currSquare + numSquares - 3) % numSquares;
				break;
			default:
				newSquare = card;
				break;
		}
		return newSquare;
	}
};

void runSimulation(int numRolls, int numDiceSides, std::array<std::pair<int, int>, numSquares>&observed) {
	auto seed = std::random_device();
	std::mt19937 gen(seed());		// pseudorandom number generator
	std::uniform_int_distribution<int> dist(1, numDiceSides);

	// Initialize and shuffle decks
	CommunityChest communityChestDeck(gen);
	Chance chanceDeck(gen);
	int square = GO;	// start on GO square

	int numDoubles = 0;
	for (int roll = 0; roll < numRolls; roll++) {
		int die1 = dist(gen);
		int die2 = dist(gen);
		if (die1 == die2) {
			if (++numDoubles == 3) {
				square = JAIL;
				observed[square].second++;
				numDoubles = 0;
				continue;
			}
		} else numDoubles = 0;
		square += (die1+die2);
		square %= numSquares;
		switch (square) {
			case G2J:
				square = JAIL;
				break;
			case CH1: case CH2: case CH3:
				square = chanceDeck.drawCard(square);
				if (square == CC3) square = communityChestDeck.drawCard(square);	// back 3 can send you to a community chest
				break;
			case CC1: case CC2: case CC3:
				square = communityChestDeck.drawCard(square);
				break;
		}
		observed[square].second++;
	}
}

std::string mostPopularMonopolySquares(int numPopularSquares, int numDiceSides) {

	// Record the number of occurrences of landing on each square
	std::array<std::pair<int, int>, numSquares> observed;
	for (int i = 0; i < numSquares; i++)
		observed[i] = {i, 0};

	// Run simulations
	constexpr int numRolls = 1'000;
	constexpr int numSimulations = 1'000;
	for (int s = 0; s < numSimulations; s++) {
		runSimulation(numRolls, numDiceSides, observed);
	}

	// Sort in descending order and print the result
	std::sort(observed.begin(), observed.end(), [](std::pair<int, int> lhs, std::pair<int, int> rhs) { return lhs.second > rhs.second; });
	std::string result = "";
	for (int i = 0; i < numPopularSquares; i++) {
		int square = observed[i].first;
		int occurrences = observed[i].second;
		std::cout << "Square " << square << ":\t" << occurrences << "/" << numRolls;
		std::cout << "\t(" << (double)occurrences/(numRolls*numSimulations)*100 << "%)\n";
		result += (square < 10 ? "0" : "");
		result += std::to_string(square);
	}

	return result;
}
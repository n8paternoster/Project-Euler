#include "stdafx.h"
#include "51-75/54.h"
#include <fstream>
#include <string>
#include <sstream>
#include <array>
#include <vector>
#include <algorithm>

const size_t handSize = 5;

enum class pokerRank {
	High_Card, One_Pair, Two_Pairs, Three_of_a_Kind, Straight, Flush, Full_House, Four_of_a_Kind, Straight_Flush, Royal_Flush
};

std::ostream& operator<<(std::ostream& os, pokerRank rank) {
	const std::string name[] = {"High Card", "One Pair", "Two Pair", "Three of a Kind", "Straight", "Flush", "Full House", "Four of a Kind", "Straight Flush", "Royal Flush"};
	return os << name[static_cast<int>(rank)];
}

enum class pokerSuit {
	Clubs, Diamonds, Hearts, Spades
};

struct pokerCard {
	int value;
	pokerSuit suit;
	pokerCard(): value(0), suit(pokerSuit::Clubs) { }
	pokerCard(char v, char s) {
		if (v >= '2' && v <= '9') value = v - '0';
		else switch (v) {
			case 'T': value = 10; break;
			case 'J': value = 11; break;
			case 'Q': value = 12; break;
			case 'K': value = 13; break;
			case 'A': value = 14; break;
			default: throw std::invalid_argument("");
		}
		switch (s) {
			case 'C': suit = pokerSuit::Clubs; break;
			case 'D': suit = pokerSuit::Diamonds; break;
			case 'H': suit = pokerSuit::Hearts; break;
			case 'S': suit = pokerSuit::Spades; break;
			default: throw std::invalid_argument("");
		}
	}
	bool operator<(const pokerCard& other) const { return value < other.value; }
	bool operator==(const pokerCard& other) const { return value == other.value; }
	bool operator!=(const pokerCard& other) const { return value != other.value; }
};

struct pokerHand {
	std::array<pokerCard, handSize> cards = {};
	std::array<int, 15> cardGroups = {};	// indexes 2-14 represent cards 2 through ace, value is the # of those cards
	bool isFlush = false;
	bool isStraight = false;
	pokerRank rank = pokerRank::High_Card;
	bool operator<(const pokerHand& other) const {	// assumes cards are sorted
		if (rank != other.rank) return rank < other.rank;
		// compare the ranks without kickers
		int groupOneSize = 0, groupTwoSize = 0;
		switch (rank) {	
			case pokerRank::Royal_Flush: return false;
			case pokerRank::Straight_Flush:
			case pokerRank::Straight: return (cards.back() < other.cards.back());
			case pokerRank::Four_of_a_Kind: groupOneSize = 4; break;
			case pokerRank::Full_House: groupTwoSize = 2;
			case pokerRank::Three_of_a_Kind: groupOneSize = 3; break;
			case pokerRank::Two_Pairs: groupTwoSize = 2;
			case pokerRank::One_Pair: groupOneSize = 2; break;
		}
		if (groupOneSize) {
			int groupValue = cardGroups.size()-1 - (std::find(cardGroups.rbegin(), cardGroups.rend(), groupOneSize) - cardGroups.rbegin());
			int otherGroupValue = other.cardGroups.size()-1 - (std::find(other.cardGroups.rbegin(), other.cardGroups.rend(), groupOneSize) - other.cardGroups.rbegin());
			if (groupValue != otherGroupValue) return groupValue < otherGroupValue;
		}
		if (groupTwoSize) {
			int groupValue = std::find(cardGroups.begin(), cardGroups.end(), groupTwoSize) - cardGroups.begin();
			int otherGroupValue = std::find(other.cardGroups.begin(), other.cardGroups.end(), groupTwoSize) - other.cardGroups.begin();
			if (groupValue != otherGroupValue) return groupValue < otherGroupValue;
		}
		// ranks are equal, compare with kickers
		return std::lexicographical_compare(cards.rbegin(), cards.rend(), other.cards.rbegin(), other.cards.rend());
	}
	bool operator==(const pokerHand& other) const {	// assumes cards are sorted
		if (rank != other.rank) return false;
		else return std::equal(cards.begin(), cards.end(), other.cards.begin());
	}
};


// returns 1 for player 1 victory, 2 for player 2 victory, and 0 otherwise
int handWinner(std::vector<pokerHand> hands) {

	for (auto &hand : hands)
		std::sort(hand.cards.begin(), hand.cards.end());

	// Check hands for flushes and straights
	for (auto &hand : hands) {
		hand.isFlush = true;
		hand.isStraight = true;
		pokerSuit flushSuit = hand.cards[0].suit;
		int straightHigh = hand.cards[0].value;
		for (size_t i = 1; i < handSize; i++) {
			if (hand.cards[i].suit != flushSuit) hand.isFlush = false;
			if (hand.cards[i].value-1 != straightHigh++) hand.isStraight = false;
			if (!hand.isFlush && !hand.isStraight) break;
		}
	}

	// Get the best rank for each hand
	for (auto &hand : hands) {
		if (hand.isStraight) {
			if (hand.isFlush) {
				if (hand.cards.back().value == 14) hand.rank = pokerRank::Royal_Flush;
				else hand.rank = pokerRank::Straight_Flush;
			} else hand.rank = pokerRank::Straight;
		} else if (hand.isFlush) {
			hand.rank = pokerRank::Flush;
		} else {
			// check for a group (One_Pair, Two_Pairs, Three_of_a_Kind, Four_of_a_Kind, Full_House)
			hand.cardGroups.fill(0);
			for (auto const &card : hand.cards)
				hand.cardGroups[card.value]++;
			for (auto groupSize : hand.cardGroups) {
				switch (groupSize) {
					case 2: if (hand.rank == pokerRank::One_Pair) hand.rank = pokerRank::Two_Pairs;
							else if (hand.rank == pokerRank::Three_of_a_Kind) hand.rank = pokerRank::Full_House;
							else hand.rank = pokerRank::One_Pair;
							break;
					case 3: if (hand.rank == pokerRank::One_Pair) hand.rank = pokerRank::Full_House;
							else hand.rank = pokerRank::Three_of_a_Kind;
							break;
					case 4: hand.rank = pokerRank::Four_of_a_Kind;
				}
			}
		}
	}

	// Compare hands
	std::vector<size_t> winningPlayers;
	auto it = std::max_element(hands.begin(), hands.end());
	if (std::count(hands.begin(), hands.end(), *it) > 1) return 0;
	return (it - hands.begin()) + 1;
}

unsigned numPlayer1Wins(std::string file) {

	// an array of poker games where each game contains a number of hands equal to the number of players
	std::vector<std::vector<pokerHand>> games;

	std::ifstream f(file);
	if (!f) {
		std::cerr << "Error opening file\n";
		return 0;
	}

	for (std::string game; std::getline(f, game); ) {	// for each poker game
		std::vector<pokerHand> hands;
		std::stringstream ss(game);
		for (std::string cardString; ss >> cardString; ) {	// read in every hand card by card
			pokerHand hand;
			hand.cards[0] = pokerCard(cardString[0], cardString[1]);
			for (size_t i = 1; i < handSize && ss >> cardString; i++)
				hand.cards[i] = pokerCard(cardString[0], cardString[1]);
			hands.push_back(hand);
		}
		games.push_back(hands);
	}

	f.close();

	unsigned p1Wins = 0;
	for (auto game : games)
		if (handWinner(game) == 1) p1Wins++;

	return p1Wins;
}
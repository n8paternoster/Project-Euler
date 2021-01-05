#include "stdafx.h"
#include "54.h"
#include <fstream>
#include <string>
#include <sstream>
#include <array>
#include <vector>
#include <algorithm>
#include <bitset>

const size_t handSize = 5;

enum pokerRank {
	High_Card, One_Pair = 4*handSize, Two_Pairs, Three_of_a_Kind, Straight, Flush, Full_House, Four_of_a_Kind, Straight_Flush, Royal_Flush
};

std::ostream& operator<<(std::ostream& os, pokerRank rank) {
	const std::string name[] = {"High Card", "One Pair", "Two Pair", "Three of a Kind", "Straight", "Flush", "Full House", "Four of a Kind", "Straight Flush", "Royal Flush"};
	return os << name[rank];
}

enum pokerSuit {
	Clubs, Diamonds, Hearts, Spades
};

struct pokerCard {
	int value;
	pokerSuit suit;
	pokerCard(): value(0), suit(Clubs) { }
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
			case 'C': suit = Clubs; break;
			case 'D': suit = Diamonds; break;
			case 'H': suit = Hearts; break;
			case 'S': suit = Spades; break;
			default: throw std::invalid_argument("");
		}
	}
	bool operator<(const pokerCard& other) const { return value < other.value; }
	bool operator==(const pokerCard& other) const { return value == other.value; }
	bool operator!=(const pokerCard& other) const { return value != other.value; }
};

struct pokerHand {
	std::array<pokerCard, handSize> cards = {};
	bool isFlush = false;
	bool isStraight = false;
	std::array<int, 15> cardGroups = {};	// indexes 2-14 represent cards 2 through ace, value is the # of those cards
	pokerRank rank = High_Card;
	std::bitset<8+4*handSize> score;	// 8 bits for rank, 4 for each card
	//bool operator<(const pokerHand& other) const {	// assumes cards are sorted
	//	if (rank != other.rank) return rank < other.rank;
	//	// compare the ranks without kickers
	//	int groupOneSize = 0, groupTwoSize = 0;
	//	switch (rank) {	
	//		case Royal_Flush: return false;
	//		case Straight_Flush:
	//		case Straight: return (cards.back() < other.cards.back());
	//		case Four_of_a_Kind: groupOneSize = 4; break;
	//		case Full_House: groupTwoSize = 2;
	//		case Three_of_a_Kind: groupOneSize = 3; break;
	//		case Two_Pairs: groupTwoSize = 2;
	//		case One_Pair: groupOneSize = 2; break;
	//	}
	//	if (groupOneSize) {
	//		int groupValue = cardGroups.size()-1 - (std::find(cardGroups.rbegin(), cardGroups.rend(), groupOneSize) - cardGroups.rbegin());
	//		int otherGroupValue = other.cardGroups.size()-1 - (std::find(other.cardGroups.rbegin(), other.cardGroups.rend(), groupOneSize) - other.cardGroups.rbegin());
	//		if (groupValue != otherGroupValue) return groupValue < otherGroupValue;
	//	}
	//	if (groupTwoSize) {
	//		int groupValue = std::find(cardGroups.begin(), cardGroups.end(), groupTwoSize) - cardGroups.begin();
	//		int otherGroupValue = std::find(other.cardGroups.begin(), other.cardGroups.end(), groupTwoSize) - other.cardGroups.begin();
	//		if (groupValue != otherGroupValue) return groupValue < otherGroupValue;
	//	}
	//	// ranks are equal, compare with kickers
	//	return std::lexicographical_compare(cards.rbegin(), cards.rend(), other.cards.rbegin(), other.cards.rend());
	//}
	//bool operator==(const pokerHand& other) const {	// assumes cards are sorted
	//	if (rank != other.rank) return false;
	//	else return std::equal(cards.begin(), cards.end(), other.cards.begin());
	//}
	bool operator<(const pokerHand &other) const {
		std::string scoreStr = score.to_string();
		std::string otherScoreStr = other.score.to_string();
		return std::lexicographical_compare(scoreStr.begin(), scoreStr.end(), otherScoreStr.begin(), otherScoreStr.end());
	}
	bool operator==(const pokerHand &other) const {
		return score == other.score;
	}
};

void scoreHand(pokerHand &hand) {
	hand.score.reset();
	std::sort(hand.cards.begin(), hand.cards.end());

	auto setCardScore = [&](size_t cardIndex, int value) {
		size_t i = cardIndex*4;
		while (value != 0) {
			hand.score.set(i++, (value%2 != 0));
			value /= 2;
		}
	};

	if (std::all_of(hand.cards.begin(), hand.cards.end(), [&](pokerCard c) { return c.suit==hand.cards[0].suit; })) {
		if (hand.cards.back().value - hand.cards.front().value == handSize-1) {	// straight flush
			hand.score.set(Straight_Flush);
		} else {	// flush
			hand.score.set(Flush);
		}
	} else {
		auto it1 = std::adjacent_find(hand.cards.rbegin(), hand.cards.rend());
		if (it1 != hand.cards.rend()) {
			auto count1 = std::count(hand.cards.begin(), hand.cards.end(), *it1);
			auto it2 = std::adjacent_find(it1+count1, hand.cards.rend());
			if (it2 != hand.cards.rend()) {
				auto count2 = std::count(hand.cards.begin(), hand.cards.end(), *it2);
				if (count1 == 2 && count2 == 2) {		// two pair
					hand.score.set(Two_Pairs);
					for (size_t i = 1; i <= 2; i++) setCardScore(handSize-i, (*it1).value);
					for (size_t i = 3; i <= 4; i++) setCardScore(handSize-i, (*it2).value);
				} else if (count1 == 3 && count2 == 2) {// full house
					hand.score.set(Full_House);
					for (size_t i = 1; i <= 3; i++) setCardScore(handSize-i, (*it1).value);
					for (size_t i = 4; i <= 5; i++) setCardScore(handSize-i, (*it2).value);
				} else if (count1 == 2 && count2 == 3) {// full house
					hand.score.set(Full_House);
					for (size_t i = 1; i <= 3; i++) setCardScore(handSize-i, (*it2).value);
					for (size_t i = 4; i <= 5; i++) setCardScore(handSize-i, (*it1).value);
				}
			} else {
				if (count1 == 2) {			// pair
					hand.score.set(One_Pair);
					for (size_t i = 1; i <= 2; i++) setCardScore(handSize-i, (*it1).value);
				} else if (count1 == 3) {	// three of a kind
					hand.score.set(Three_of_a_Kind);
					for (size_t i = 1; i <= 3; i++) setCardScore(handSize-i, (*it1).value);
				} else if (count1 == 4) {	// four of a kind
					hand.score.set(Four_of_a_Kind);
					for (size_t i = 1; i <= 4; i++) setCardScore(handSize-i, (*it1).value);
				}
			}
		} else if (hand.cards.back().value - hand.cards.front().value == handSize-1) {	// straight
			hand.score.set(Straight);
		}
	}

	// Add the remaining cards to the score
	for (size_t i = 0, j = 0; i < handSize; i++) {
		if (std::count(hand.cards.begin(), hand.cards.end(), hand.cards[i]) == 1) {
			setCardScore(j++, hand.cards[i].value);
		}
	}
}

// returns a list of integers representing winning players (1 for player 1, etc)
std::vector<size_t> handWinner(std::vector<pokerHand> hands) {

	for (auto &hand : hands) {
		std::sort(hand.cards.begin(), hand.cards.end());
		scoreHand(hand);
	}

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
				if (hand.cards.back().value == 14) hand.rank = Royal_Flush;
				else hand.rank = Straight_Flush;
			} else hand.rank = Straight;
		} else if (hand.isFlush) {
			hand.rank = Flush;
		} else {
			// check for a group (One_Pair, Two_Pairs, Three_of_a_Kind, Four_of_a_Kind, Full_House)
			hand.cardGroups.fill(0);
			for (auto const &card : hand.cards)
				hand.cardGroups[card.value]++;
			for (auto groupSize : hand.cardGroups) {
				switch (groupSize) {
					case 2: if (hand.rank == One_Pair) hand.rank = Two_Pairs;
							else if (hand.rank == Three_of_a_Kind) hand.rank = Full_House;
							else hand.rank = One_Pair;
							break;
					case 3: if (hand.rank == One_Pair) hand.rank = Full_House;
							else hand.rank = Three_of_a_Kind;
							break;
					case 4: hand.rank = Four_of_a_Kind;
				}
			}
		}
	}

	// Compare hands
	std::vector<size_t> winningPlayers;
	auto it = std::max_element(hands.begin(), hands.end());
	for (size_t i = 0; i < hands.size(); i++) {
		if (i == it - hands.begin()) winningPlayers.push_back(i+1);
		else if (hands[i] == *it) winningPlayers.push_back(i+1);
	}

	return winningPlayers;
}

std::vector<size_t> newHandWinner(std::vector<pokerHand> hands) {

	// score hands
	for (auto &hand : hands)
		scoreHand(hand);

	// compare hands
	std::vector<size_t> winningPlayers;
	auto it = std::max_element(hands.begin(), hands.end());
	for (size_t i = 0; i < hands.size(); i++) {
		if (i == it - hands.begin()) winningPlayers.push_back(i+1);
		else if (hands[i] == *it) winningPlayers.push_back(i+1);
	}

	return winningPlayers;
}

unsigned numPlayer1Wins() {

	// an array of poker games where each game contains a number of hands equal to the number of players
	std::vector<std::vector<pokerHand>> games;

	std::ifstream f("54.txt");
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
	for (auto game : games) {
		//std::vector<size_t> winners = handWinner(game);
		std::vector<size_t> winners = newHandWinner(game);
		if (std::find(winners.begin(), winners.end(), 1) != winners.end()) p1Wins++;
	}

	return p1Wins;
}
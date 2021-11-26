#include "pch.h"
#include "51-75/61.h"
#include <vector>
#include <set>
#include <map>
#include <algorithm>

const unsigned numShapes = 6;
unsigned numDigits = 4;

struct node {
	unsigned s;		// 3 = triangular, 4 = square, 5 = pentagonal, etc
	unsigned p;		// polygonal number
	std::vector<node> adjNodes;
	node(): s(0), p(0), adjNodes() { }
	node(unsigned S, unsigned P): s(S), p(P), adjNodes() { }
	bool hasEdge(const node &n) const { return std::count(adjNodes.begin(), adjNodes.end(), n) > 0; }
};

bool operator<(const node &l, const node &r) {
	if (l.s < r.s) return true;
	else if (l.s > r.s) return false;
	if (l.p < r.p) return true;
	else if (l.p > r.p) return false;
	return false;
}

bool operator==(const node &l, const node &r) {
	return (l.s == r.s && l.p == r.p);
}

class graph {
	void DFS(std::vector<node> &path, std::set<unsigned> &sVals, std::map<node, bool> &visited, const node &startNode, const node &currentNode, unsigned cycleLength, unsigned currentNodeNumber = 1);
	std::vector<node> nodes;
	std::vector<std::vector<node>> cycles;
public:
	graph(): nodes(), cycles() { }
	graph(const std::map<unsigned, std::vector<unsigned>> &polygonals);
	bool addNode(unsigned s, unsigned p) {
		if (std::find(nodes.begin(), nodes.end(), node(s, p)) != nodes.end()) return false;
		nodes.push_back(node(s, p));
		return true;
	}
	void addEdge(const node &from, const node &to) {
		if (std::find(nodes.begin(), nodes.end(), from) == nodes.end()) nodes.push_back(from);
		if (std::find(nodes.begin(), nodes.end(), to) == nodes.end()) nodes.push_back(to);
		std::find(nodes.begin(), nodes.end(), from)->adjNodes.push_back(to);
	}
	std::vector<std::vector<node>> findCycles(unsigned cycleLength);
	void printNodes() const;
	void printCycles() const;
};

inline bool isCyclic(unsigned p, unsigned q) {
	static unsigned pFactor = (unsigned)pow(10, numDigits/2 + (numDigits%2 != 0));	// ceiling(numDigits/2)
	static unsigned qFactor = (unsigned)pow(10, numDigits/2);
	return (p%pFactor == q/qFactor);
}

graph::graph(const std::map<unsigned, std::vector<unsigned>> &polygonals): nodes(), cycles() {
	for (auto v : polygonals) {
		unsigned s = v.first;
		for (auto p : v.second) {

			// Check every polygonal number p against every other polygonal number of a different s
			for (unsigned otherS = 3; otherS <= 8; otherS++) {
				if (otherS == s) continue;
				for (auto otherP : polygonals.at(otherS))
					if (isCyclic(p, otherP))
						addEdge(node(s, p), node(otherS, otherP));
			}
		}
	}
}

void graph::printNodes() const {
	for (auto &n : nodes) {
		std::cout << "Node: (s = " << n.s << ", p = " << n.p << ")\tAdj nodes: ";
		for (auto &adj : n.adjNodes)
			std::cout << "(" << adj.s << ", " << adj.p << ") ";
		std::cout << "\n";
	}
}

void graph::printCycles() const {
	unsigned counter = 1;
	for (auto &cycle : cycles) {
		std::cout << "Cycle " << counter << ": ";
		for (auto &n : cycle)
			std::cout << "(" << n.s << ", " << n.p << ") ";
		std::cout << "\n";
		counter++;
	}
}

void graph::DFS(std::vector<node> &path, std::set<unsigned> &sVals, std::map<node, bool> &visited, const node &startNode, const node &currentNode, unsigned cycleLength, unsigned currentNodeNumber) {

	visited[currentNode] = true;
	
	if (currentNodeNumber == cycleLength) {
		visited[currentNode] = false;
		if (std::find(nodes.begin(), nodes.end(), currentNode)->hasEdge(startNode)) {

			// cycle found
			std::vector<node> cycle;
			cycle.push_back(startNode);
			for (auto &n : path) cycle.push_back(n);
			auto startIt = cycle.begin();
			for (auto it = cycle.begin(); it != cycle.end(); ++it)
				if (it->s < startIt->s) startIt = it;
			std::rotate(cycle.begin(), startIt, cycle.end());
			cycles.push_back(cycle);
			return;
		} else
			return;
	}

	for (auto &adjNode : std::find(nodes.begin(), nodes.end(), currentNode)->adjNodes) {
		if (!visited[adjNode] && sVals.insert(adjNode.s).second) {
			path.push_back(adjNode);
			DFS(path, sVals, visited, startNode, adjNode, cycleLength, currentNodeNumber+1);
			path.pop_back();
			sVals.erase(adjNode.s);
		}
	}

	visited[currentNode] = false;
}

std::vector<std::vector<node>> graph::findCycles(unsigned cycleLength) {
	std::map<node, bool> visited;
	for (auto &node : nodes)
		visited[node] = false;

	std::sort(nodes.begin(), nodes.end());
	
	// to find every cycle of a specified length, we only need to search a limited number of starting nodes to guarantee every possible cycle has been checked
	// since there are fewer high-s polygonals, search starting from those
	unsigned sMin = numShapes+2 - (numShapes - cycleLength);
	for (auto &start : nodes) {
		if (start.s >= sMin) {
			std::vector<node> path;
			std::set<unsigned> sVals = {start.s};
			DFS(path, sVals, visited, start, start, cycleLength);
		}
	}

	// remove duplicate cycles
	std::sort(cycles.begin(), cycles.end());
	auto it = std::unique(cycles.begin(), cycles.end());
	cycles.resize(std::distance(cycles.begin(), it));

	return cycles;
}

unsigned sumOfCyclicPolygonalNums(unsigned nDigits, unsigned cycleLength) {
	if (nDigits < 2 || nDigits > 9) return 0;					// avoid overflow
	if (cycleLength < 2 || cycleLength > numShapes) return 0;	// cycle must contain only one number per shape						
	numDigits = nDigits;

	// Store all n-digit polygonal numbers
	std::map<unsigned, std::vector<unsigned>> polygonals;
	const unsigned min = (unsigned)pow(10, numDigits-1);
	const unsigned max = (unsigned)pow(10, numDigits)-1;
	for (unsigned s = 3; s < 3+numShapes; s++) {
		unsigned n = 0;
		unsigned p = 0;
		while (p <= max) {
			n++;
			p = ((s-2)*n*n - (s-4)*n) / 2;
			if (p >= min && p <= max) polygonals[s].push_back(p);
		}
	}

	graph g(polygonals);
	std::vector<std::vector<node>> cycles = g.findCycles(cycleLength);
	g.printCycles();

	unsigned sum = 0;
	for (auto cycle : cycles)
		for (auto n : cycle)
			sum += n.p;
	return sum;
}

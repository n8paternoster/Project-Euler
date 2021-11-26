#include "stdafx.h"
#include "76-100/81.h"
#include <string>
#include <sstream>
#include <fstream>
#include <queue>	// std::priority_queue

int dijkstraTwoWays(int from, int to, int *weight, const int N) {
	
	// Distance to each node from the source node
	int *dist = new int[N*N]();	
	for (int i = 0; i < N*N; i++)
		dist[i] = INT_MAX;
	dist[from] = weight[from];	// initial node

	// Min-priority queue with custom comparison fnc
	auto comp = [&dist](int lhs, int rhs) -> bool { return dist[lhs] > dist[rhs]; };
	std::priority_queue<int, std::vector<int>, decltype(comp)> queue(comp);
	
	queue.push(from);
	while (!queue.empty()) {
		int u = queue.top();
		queue.pop();
		if (u == to) break;
		int row = u / N, col = u % N;
		int neighbors[2] = {col+1 < N ? u+1 : -1,		// right
							row+1 < N ? u+N : -1};		// bottom
		for (int i = 0; i < 2; i++) {
			int v = neighbors[i];
			if (v != -1) {
				int alt = dist[u] + weight[v];
				if (alt < dist[v]) {
					dist[v] = alt;
					queue.push(v);
				}
			}
		}
	}
	int result = dist[to];
	delete[] dist;
	return result;
}

void readMatrix(std::string file, int *data, const int N) {
	std::ifstream f(file);
	if (!f) {
		std::cerr << "Error opening file";
		return;
	}
	for (int x = 0; x < N; x++) {
		// Rows separated by '\n'
		std::string buff;
		f >> buff;
		std::stringstream row(buff);
		for (int y = 0; y < N; y++) {
			// Cols separated by ','
			std::string val;
			std::getline(row, val, ',');
			data[x*N+y] = std::stoi(val);
		}
	}
}

int minPathTwoWays(std::string file, const int N) {

	// Read the data
	int *matrix = new int[N*N]();
	readMatrix(file, matrix, N);

	// Find the shortest path from top left (0, 0) to bottom right (N-1, N-1)
	int result = dijkstraTwoWays(0, N*N-1, matrix, N);

	delete[] matrix;
	return result;
}
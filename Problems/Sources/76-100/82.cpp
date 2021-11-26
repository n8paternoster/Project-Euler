#include "pch.h"
#include "76-100/82.h"
#include "76-100/81.h"		// readMatrix()
#include <string>
#include <sstream>
#include <fstream>
#include <queue>	// std::priority_queue

int dijkstraThreeWays(int fromCol, int toCol, int *weight, const int N) {

	// Distance to each node from the source node
	int *dist = new int[N*N]();
	for (int i = 0; i < N*N; i++)
		dist[i] = INT_MAX;

	// Min-priority queue with custom comparison fnc
	auto comp = [&dist](int lhs, int rhs) -> bool { return dist[lhs] > dist[rhs]; };
	std::priority_queue<int, std::vector<int>, decltype(comp)> queue(comp);

	// The source 'node' is some node with zero weight that has all nodes in the 'from' col as right neighbors, add all these nodes to the queue
	for (int r = 0; r < N; r++) {
		int node = r*N + fromCol;
		dist[node] = weight[node];
		queue.push(node);
	}

	// Find the minimum paths to each node
	while (!queue.empty()) {
		int u = queue.top();
		queue.pop();
		int row = u / N, col = u % N;
		int neighbors[3] = {col+1 < N ? u+1 : -1,		// right
							row+1 < N ? u+N : -1,		// bottom
							row-1 >= 0 ? u-N : -1};		// top
		for (int i = 0; i < 3; i++) {
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

	// Check every end node in the 'to' col
	int minPath = INT_MAX;
	for (int endRow = 0; endRow < N; endRow++) {
		int dest = endRow*N + toCol;
		if (dist[dest] < minPath) minPath = dist[dest];
	}

	delete[] dist;
	return minPath;
}

int minPathThreeWays(std::string file, const int N) {

	// Read the data
	int *matrix = new int[N*N]();
	readMatrix(file, matrix, N);

	// Find the shortest path from the left (0) col to the right (N-1) col
	int result = dijkstraThreeWays(0, N-1, matrix, N);

	delete[] matrix;
	return result;
}
#include "stdafx.h"
#include "76-100/78.h"
#include <vector>

/* Uses the same formula from problem 76 with the only difference being reducing the partition by modulo m
*/
int numPartitionsOfNModM(std::vector<int>& p, int n, int m) {
    if (n < 0) return 0;
    if (p[n] != 0) return p[n];
    int count = 0, k = 1, gk = 1;
    while (n - gk >= 0) {
        count += numPartitionsOfNModM(p, n-gk, m) * (k % 2 == 0 ? -1 : 1);
        k *= -1;
        if (k > 0) k++;
        gk = (k*(3*k-1)) / 2;
    }
    p[n] = count % m;
    return count % m;
}

int lowestPartitionDivisbleBy(int m) {

    std::vector<int> p; // partitions modulo m
    p.push_back(1%m);   // base case, p[0] = 1
    int n = 0;
    while (p[n] != 0) {
        n++;
        p.push_back(0);
        numPartitionsOfNModM(p, n, m);
    }
    return n;
}

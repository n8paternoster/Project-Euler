#include "stdafx.h"
#include "76-100/76.h"

int countPartitions(int *p, int n) {
    if (n < 0) return 0;
    if (p[n]) return p[n];
    int count = 0, k = 1, gk = 1;
    while (n - gk >= 0) {
        count += countPartitions(p, n-gk) * (k % 2 == 0 ? -1 : 1);
        k *= -1;
        if (k > 0) k++;
        gk = (k*(3*k-1)) / 2;
    }
    p[n] = count;
    return count;
}

int numPartitions(int n) {
    int *p = new int[n+1]();
    p[0] = 1;   // base case
    int result = countPartitions(p, n);
    delete[] p;
    return result-1;    // exclude the partition of just 'n'
}

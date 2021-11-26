#include "stdafx.h"
#include "51-75/75.h"

struct pyTriple {
    int a;
    int b;
    int c;
};

std::ostream& operator<<(std::ostream& os, const pyTriple& pyT) {
    std::cout << "("<< pyT.a << ", " << pyT.b << ", " << pyT.c << ") = " << pyT.a+pyT.b+pyT.c;
    return os;
}

// Generates all pythagorean triples 
void generateTriples(pyTriple node, int *sumCount, int maxSum) {

    int sum = node.a + node.b + node.c;

    // The sum of a node's children will always be larger, stop traversing the tree
    if (sum > maxSum) return;

    // Record the sum of this primitive triple and all its multiples
    int k = 1;
    while (k*sum < maxSum) sumCount[sum*k++]++;

    // Generate the three children of this node, all of which are primitive
    pyTriple leftChild;
    leftChild.a     =  1*node.a + -2*node.b + 2*node.c;
    leftChild.b     =  2*node.a + -1*node.b + 2*node.c;
    leftChild.c     =  2*node.a + -2*node.b + 3*node.c;
    generateTriples(leftChild, sumCount, maxSum);

    pyTriple middleChild;
    middleChild.a   =  1*node.a +  2*node.b + 2*node.c;
    middleChild.b   =  2*node.a +  1*node.b + 2*node.c;
    middleChild.c   =  2*node.a +  2*node.b + 3*node.c;
    generateTriples(middleChild, sumCount, maxSum);

    pyTriple rightChild;
    rightChild.a    = -1*node.a +  2*node.b + 2*node.c;
    rightChild.b    = -2*node.a +  1*node.b + 2*node.c;
    rightChild.c    = -2*node.a +  2*node.b + 3*node.c;
    generateTriples(rightChild, sumCount, maxSum);
}

int numSinglePythagoreanTripleSums(int maxSum) {
    int *sumCount = new int[maxSum+1]();
    generateTriples({3, 4, 5}, sumCount, maxSum);   // initial node of the tree is the triple {3, 4, 5}
    int count = 0;
    for (int sum = 12; sum <= maxSum; sum += 2)
        if (sumCount[sum] == 1) count++;
    delete[] sumCount;
    return count;
}

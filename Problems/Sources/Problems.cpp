// Problems.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <chrono>
#include "bigNum.h"
#include "76-100/91.h"

int main()
{
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

    std::cout << p91();

    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> deltaT = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    std::cout << "\nElapsed time: " << deltaT.count() << " seconds\n";
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

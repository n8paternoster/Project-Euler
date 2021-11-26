#include "stdafx.h"
#include "76-100/96.h"
#include <string>
#include <fstream>
#include <windows.h>
#include "sudokuBoardAPI.h"

// A factory function to create a sudokuBoard instance
typedef sudokuBoardAPI* (__cdecl *sudokuBoardAPI_factory)();

/* Solved this problem using my own sudoku solving app, loaded here dynamically in a dll.
* This app solves sudoku puzzles using pencil-and-paper techniques including some fairly advanced solving strategies listed on https://www.sudokuwiki.org/Strategy_Families
*/
int sudokuSolutionSum(std::string file) {

    // Open the text file
    std::ifstream f(file);
    if (!f) {
        std::cerr << "Error opening file";
        return 0;
    }

    // Load the DLL into memory and get its address
    HINSTANCE dll_handle = ::LoadLibrary(TEXT("Sudoku Board.dll"));
    if (!dll_handle) {
        std::cerr << "Unable to load DLL\n";
        return 0;
    }

    // Get the function from the DLL
    sudokuBoardAPI_factory factory_func = reinterpret_cast<sudokuBoardAPI_factory>(::GetProcAddress(dll_handle, "createSudokuBoard"));
    if (!factory_func) {
        std::cout << "GetProcAddress error: " << GetLastError() << "\n";
        std::cerr << "Unable to load createSudokuBoard from DLL\n";
        ::FreeLibrary(dll_handle);
        return 0;
    }

    // Create a new object implementing the sudokuBoardAPI
    sudokuBoardAPI *board = factory_func();

    // Solve the puzzles
    std::string buff, boardData;
    int rowCount = 0, result = 0;
    while (std::getline(f, buff) || rowCount == 9) {
        if (rowCount == 9) {
            board->clear();
            board->fillData(boardData);
            board->solve();
            board->drawToConsole();
            result += 100 * (*board)(0, 0);
            result += 10 * (*board)(0, 1);
            result += (*board)(0, 2);
            boardData.clear();
            rowCount = 0;
        }
        if (buff[0] == 'G') {
            std::cout << buff << "\n";
            continue;
        }
        boardData += buff;
        rowCount++;
    }

    // Destroy the object explicitly and unload the DLL
    board->destroy();
    ::FreeLibrary(dll_handle);

    return result;
}

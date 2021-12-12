// StringCats.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
using namespace std;
int main()
{
    int turnNumber = 5;
    int xPos = 6;
    int yPos = 7;

    // serialize
    std::string s = std::to_string(turnNumber) + "," + std::to_string(xPos) + "," + std::to_string(yPos);
    std::cout << s << std::endl;

    // deserialize
    // (note this modifies the original string..)
    int retrievedTurnNumber = -1;
    int retrievedXPos = -1;
    int retrievedYPos = -1;
    // this is just one way to do this...
    vector<string> parts;
    stringstream ss(s);
    while (ss.good())
    {
        string substr;
        getline(ss, substr, ',');
        parts.push_back(substr);
    }
    retrievedTurnNumber = stoi(parts[0]);
    retrievedXPos = stoi(parts[1]);
    retrievedYPos = stoi(parts[2]);
     
    std::cout << retrievedTurnNumber << "," << retrievedXPos << "," << retrievedYPos << endl;
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

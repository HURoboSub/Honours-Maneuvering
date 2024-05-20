/*  testStateMachine.cpp
    Author: J. Bloemendal
    Template
    Date: xx-xx-2024
    Test states using a gcc or dev cpp compilor
    To compile:
        OS X build:
            g++ testStateMachine.cpp -o runme -std=c++11xw
/ // ADD (*) to start of this line TO toggle code

#include <iostream>

// Defines
// #define DEBUG               // Uncomment to enable DEBUG mode

#define LCD_ROWS 2
#define LCD_COLS 16

// enumator for the system states
enum class systemState
{
    Setup,       // S0 Init setup state
    Calibrating, // S1
    Reading,     // S2
    Testing,     // S3
    Output       // S4
} currentState;  // class storing the current system state

const char *rowOneLCD = "              "; // max 16 chars
const char *rowTwoLCD = "              "; // max 16 chars

void LCD_show(char **str);
void userInterface(systemState cState); // Displays the systemstate on the LCD scren

void LCD_show(char **str)
{
    unsigned char x, y; // y loop index

    for (y = 0; y < LCD_ROWS; y++)
    {
        for (x = 0; x < LCD_COLS; x++)
        {
            str[0][x] = rowOneLCD[x]; // copy row one to stirng buffer
            str[1][x] = rowTwoLCD[x]; // copy row two to stirng buffer
        }
    }

    for (y = 0; y < LCD_ROWS; y++, str++)
    {
        std::cout << *str << '\n';
    }
}

void userInterface(systemState cState)
{
    unsigned char y; // y loop index

    char **dispText = new char *[LCD_ROWS];

    for (y = 0; y < LCD_ROWS; y++)
    {
        dispText[y] = new char[LCD_COLS];
    }

    switch (currentState)
    {
    case systemState::Setup:
        // Setup state here
        rowOneLCD = "S: Setup       ";
        break;
    case systemState::Calibrating:
        // Calibrating state here
        rowOneLCD = "S: Calibrating ";
        break;
    case systemState::Reading:
        // S2 here
        rowOneLCD = "S: Reading     ";
        break;
    case systemState::Testing:
        // S3 Testing here
        rowOneLCD = "S: Testing     ";
        break;
    case systemState::Output:
        // S4 Output here
        rowOneLCD = "S: Output      ";
        break;
    default:
        rowOneLCD = "S: Error       ";
        rowTwoLCD = "No state passed";
        break;
    }

    // Show the initial display
    LCD_show(dispText);

    // Free allocated memory
    for (y = 0; y < LCD_ROWS; y++)
    {
        delete[] dispText[y];
    }
    delete[] dispText;
}

int main(int argc, char *argv[])
{
    // test each states
    for (int state = 0; state < 5; state++)
    {
        currentState = static_cast<systemState>(state);
        userInterface(currentState);
    }
    return 0;
}
/*
*/

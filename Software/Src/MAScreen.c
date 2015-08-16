//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
//      Copyright (C) 2015 Peter Walsh, Milford, NH 03055
//      All Rights Reserved under the MIT license as outlined below.
//
//  FILE
//      MAScreen.c
//
//  DESCRIPTION
//
//      Main screen for the Sone ultrasonic transducer control system
//
//  VERSION:    2015.03.04
//
//////////////////////////////////////////////////////////////////////////////////////////
//
//  MIT LICENSE
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of
//    this software and associated documentation files (the "Software"), to deal in
//    the Software without restriction, including without limitation the rights to
//    use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
//    of the Software, and to permit persons to whom the Software is furnished to do
//    so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//    all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
//    INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
//    PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
//    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
//    OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

#include "Screen.h"

#ifdef USE_MAIN_SCREEN

#include "SG3525.h"
#include "Setup.h"

#include <stdlib.h>

#include "Command.h"
#include "Parse.h"
#include "Serial.h"
#include "VT100.h"

#include "Debug.h"

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// Data declarations
//
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static bool PromptNeeded;           // TRUE if need to replot the prompt

//
// Static layout of the main screen
//
static char MAScreenText[] PROGMEM = "\
Status:  --- | Freq:  ---- |\r\n\
Curr  :  --- | Power:  --- |\r\n\
Vcc   : xxxx | PWM :   --- |\r\n\
Vc    : xxxx | Power:  --- |\r\n\
-------------+-------------+\r\n\
\r\n\
Freq C  : 128\\\r\n\
PowerSet: 255\\\r\n\
\r\n\
";

#define MA_COL1      8
#define MA_COL1a    10
#define MA_COL1b     7
#define MA_COL2     22
#define MA_COL2a    21

#define STATUS_ROW   1
#define STATUS_COL   MA_COL1a

#define FREQ_ROW     1
#define FREQ_COL     MA_COL2

#define CURRENT_ROW  2
#define CURRENT_COL  MA_COL1

#define POWER_ROW    2
#define POWER_COL    MA_COL2

#define VCC_ROW      3
#define VCC_COL      MA_COL1

#define PWM_ROW      3
#define PWM_COL      MA_COL2

#define VC_ROW       4
#define VC_COL       MA_COL1

#define FSET_ROW     7
#define FSET_COL    15

#define PSET_ROW     8
#define PSET_COL    15

#define DEBUG_ROW   10
#define MSG_ROW     15


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// ShowMAScreen - Display the main screen
//
// Inputs:      None.
//
// Outputs:     None.
//
void ShowMAScreen(void) {

    CursorHome;
    ClearScreen;
    PrintStringP(MAScreenText);

    PromptNeeded = true;
    UpdateMAScreen();
    }


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// PrintX10 - Print out x10 decimal value with decimal point
//
// Inputs:      Value to print
//
// Outputs:     None.
//
void PrintX10(uint16_t Value) {

    PrintD(Value/10,3);
    PrintChar('.');
    PrintChar('0' + (Value%10));
    }

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// UpdateMAScreen - Display the main screen
//
// Inputs:      None.
//
// Outputs:     None.
//
void UpdateMAScreen(void) {

    //
    // Calibration mode takes over the display
    //
    if( SG3525Set.PwrMode == PWR_CAL )
        return;

    //////////////////////////////////////////////////////////////////////////////////////
    //
    // Screen-specific display fields
    //
    CursorPos(STATUS_COL,STATUS_ROW);
    if( SG3525_IS_ON ) PrintStringP(PSTR(" On"));
    else               PrintStringP(PSTR("Off"));

    CursorPos(FREQ_COL,FREQ_ROW);
    PrintD(SG3525Curr.Freq,5);

    CursorPos(CURRENT_COL,CURRENT_ROW);
    PrintX10(SG3525Curr.Current);

    CursorPos(POWER_COL,POWER_ROW);
    PrintX10(SG3525Curr.Power);

    CursorPos(PWM_COL,PWM_ROW);
    PrintX10(SG3525Curr.PWM);

#ifdef USE_WIPER_CMDS
    CursorPos(FSET_COL,FSET_ROW);
    PrintD(SG3525Curr.FreqCWiper,5);    // == %5d

    CursorPos(PSET_COL,PSET_ROW);
    PrintX10(SG3525Curr.PWMWiper);
#else
    CursorPos(FSET_COL,FSET_ROW);
    PrintD(SG3525Curr.Freq,5);          // == %5d

    CursorPos(PSET_COL,PSET_ROW);
    PrintX10(SG3525Set.Power);
#endif

    CursorPos(1,DEBUG_ROW);
    DebugPrint();

    //
    //
    //
    //////////////////////////////////////////////////////////////////////////////////////

    if( PromptNeeded )
        Prompt();
    PromptNeeded = false;
    
    PlotInput();
    }

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// MAScreenCommand
//
// Inputs:      Command entered at screen
//
// Outputs:     TRUE  if we understood and processed command
//              FALSE if command isn't ours, belongs to another system
//
#ifdef USE_MAIN_SCREEN_CMDS

bool MAScreenCommand(char *Command) {

    if( SG3525Cmd(Command) )
        return true;

    if( SetupCmd(Command) )
        return true;

    //
    // CL - Clear the message area
    //
    if( StrEQ(Command,"CL") ) {
        StartMsg();
        return true;
        }

    //
    // Command was unrecognized by this panel - caller must take responsibility
    //
    return false;
    }


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// StartMsg - Setup for message reporting
//
// Inputs:      None.
//
// Outputs:     None.
//
void StartMsg(void) {

    CursorPos(1,MSG_ROW);
    ClearEOS;

    PromptNeeded = true;
    }


#endif // USE_MAIN_SCREEN_CMDS

#endif // USE_MAIN_SCREEN

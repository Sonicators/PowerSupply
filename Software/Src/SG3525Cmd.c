//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
//      Copyright (C) 2015 Peter Walsh, Milford, NH 03055
//      All Rights Reserved under the MIT license as outlined below.
//
//  FILE
//      SG3525Cmd.c - Command interface to the SG3525 SMPS controller
//
//  SYNOPSIS
//
//      See SG3525.h for details
//
//  DESCRIPTION
//
//      The SG3525 module was getting a little big, so the command processing was
//        moved to this file.
//
//  VERSION:    2015.05.22
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

#include <stdlib.h>

#include "SG3525.h"

#include "Command.h"
#include "Parse.h"
#include "Serial.h"
#include "Inputs.h"
#include "EEPROM.h"
#include "MAScreen.h"

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// Data declarations
//
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// SG3525Cmd - Manage typed commands aimed at the SG3525 system
//
// Inputs:      Command to interpret
//
// Outputs:     TRUE  if we understood and processed command
//              FALSE if command isn't ours, belongs to another system
//
bool SG3525Cmd(char *Command) {

    //
    // OFF - Turn transducer output off
    //
    if( StrEQ(Command,"OF" ) ||
        StrEQ(Command,ESC_CMD)) {
        SG3525Run(false);
StartMsg();
PrintStringP(PSTR("Transducer OFF"));
        return true;
        }


    //
    // ON - Turn transducer output on
    //
    if( StrEQ(Command,"ON") ) {
        SG3525Run(true);
StartMsg();
PrintStringP(PSTR("Transducer ON"));
        return true;
        }


    //
    // XX - Special debug command
    //
    if( StrEQ(Command,"XX" ) ) {
        return true;
        }


    //
    // FR - Set frequency
    //
    if( StrEQ(Command,"FR") ) {
        char *FreqText = ParseToken();
        int   FreqNum  = atoi(FreqText);
        if( FreqNum < SG3525_MIN_FREQ ||
            FreqNum > SG3525_MAX_FREQ ) {
            StartMsg();
            PrintStringP(PSTR("Bad or out of range frequency ("));
            PrintString(FreqText);
            PrintStringP(PSTR("), must be "));
            PrintD(SG3525_MIN_FREQ,0);
            PrintStringP(PSTR(" to "));
            PrintD(SG3525_MAX_FREQ,0);
            PrintCRLF();
            PrintStringP(PSTR("Type '?' for help\r\n"));
            return true;
            }

        SG3525Set.Freq = FreqNum;
        return true;
        }

    //
    // PO - Set power
    //
    if( StrEQ(Command,"PO") ) {
        char *PowerText = ParseToken();
        int   PowerNum  = atoi(PowerText);
        if( PowerNum < SG3525_MIN_POWER ||
            PowerNum > SG3525_MAX_POWER ) {
            StartMsg();
            PrintStringP(PSTR("Bad or out of range power ("));
            PrintString(PowerText);
            PrintStringP(PSTR("), must be "));
            PrintD(SG3525_MIN_POWER,0);
            PrintStringP(PSTR(" to "));
            PrintD(SG3525_MAX_POWER,0);
            PrintCRLF();
            PrintStringP(PSTR("Type '?' for help\r\n"));
            return true;
            }

        SG3525Set.Power = PowerNum;
        return true;
        }

#ifdef USE_ADJ_CMDS
    //////////////////////////////////////////////////////////////////////////////////////
    //
    // ADJ COMMANDS
    //
    // For debugging, allow the user to directly bump the power/freq
    //

    //
    // U - Bump the frequency up 1 notch
    //
    if( StrEQ(Command,"U" ) ) {
        FreqCPotSetWiper(++SG3525Curr.FreqCWiper);
//        SG3525SetFreq(SG3525Set.Freq+1);
        return true;
        }


    //
    // D - Bump the frequency down 1 notch
    //
    if( StrEQ(Command,"D" ) ) {
        FreqCPotSetWiper(--SG3525Curr.FreqCWiper);
//        SG3525SetFreq(SG3525Set.Freq-1);
        return true;
        }


    //
    // W - Make PWM wider
    //
    if( StrEQ(Command,"W" ) ) {
        PWMPotSetWiper(++SG3525Curr.PWMWiper);
//        SG3525SetPower(SG3525Set.Power+1);
        return true;
        }


    //
    // N - Make PWM narrower
    //
    if( StrEQ(Command,"N" ) ) {
        PWMPotSetWiper(--SG3525Curr.PWMWiper);
//        SG3525SetPower(SG3525Set.Power-1);
        return true;
        }

    //
    // + - Make frequency go up by a little
    //
    if( StrEQ(Command,"+" ) ) {
        FreqFPotSetWiper(++SG3525Curr.FreqFWiper);
//        SG3525SetPower(SG3525Set.Power+1);
        return true;
        }


    //
    // - - Make frequency go down by a little
    //
    if( StrEQ(Command,"-" ) ) {
        FreqFPotSetWiper(--SG3525Curr.FreqFWiper);
//        SG3525SetPower(SG3525Set.Power-1);
        return true;
        }

#endif // USE_ADJ_CMDS


#ifdef USE_WIPER_CMDS
    //////////////////////////////////////////////////////////////////////////////////////
    //
    // WIPER COMMANDS
    //
    // For debugging, allow the user to directly set the wiper positions
    //

    //
    // FCW - Set frequency coarse wiper
    //
    if( StrEQ(Command,"FCW") ) {
        char *FreqText = ParseToken();
        int   FreqNum  = atoi(FreqText);
        if( FreqNum < 0 ||
            FreqNum > FreqCPot_MAX_WIPER ) {
            StartMsg();
            PrintStringP(PSTR("Bad or out of range wiper ("));
            PrintString(FreqText);
            PrintStringP(PSTR("), must be 0 to "));
            PrintD(FreqCPot_MAX_WIPER,0);
            PrintCRLF();
            PrintStringP(PSTR("Type '?' for help\r\n"));
            PrintCRLF();
            return true;
            }

        SG3525Curr.FreqCWiper = FreqNum;
        FreqCPotSetWiper(SG3525Curr.FreqCWiper);
        return true;
        }


    //
    // FFW - Set frequency fine wiper
    //
    if( StrEQ(Command,"FFW") ) {
        char *FreqText = ParseToken();
        int   FreqNum  = atoi(FreqText);
        if( FreqNum < 0 ||
            FreqNum > FreqFPot_MAX_WIPER ) {
            StartMsg();
            PrintStringP(PSTR("Bad or out of range wiper ("));
            PrintString(FreqText);
            PrintStringP(PSTR("), must be 0 to "));
            PrintD(FreqFPot_MAX_WIPER,0);
            PrintCRLF();
            PrintStringP(PSTR("Type '?' for help\r\n"));
            PrintCRLF();
            return true;
            }

        SG3525Curr.FreqFWiper = FreqNum;
        FreqFPotSetWiper(SG3525Curr.FreqCWiper);
        return true;
        }


    //
    // PW - Set transducer power wiper
    //
    if( StrEQ(Command,"PW") ) {
        char *PowerText = ParseToken();
        int   PowerNum  = atoi(PowerText);
        if( PowerNum < 0 ||
            PowerNum > PWMPot_MAX_WIPER ) {
            StartMsg();
            PrintStringP(PSTR("Bad or out of range wiper ("));
            PrintString(PowerText);
            PrintStringP(PSTR("), must be 0 to "));
            PrintD(PWMPot_MAX_WIPER,0);
            PrintCRLF();
            PrintStringP(PSTR("Type '?' for help\r\n"));
            PrintCRLF();
            return true;
            }

        SG3525Curr.PWMWiper = PowerNum;
        PWMPotSetWiper(SG3525Curr.PWMWiper);
        return true;
        }
#endif // USE_WIPER_CMDS


    return false;
    }

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// ProcessInputX - Process input trigger events
//
// Inputs:      Trigger event to process (1 == button pressed)
//
// Outputs:     None.
//
// NOTE: See Inputs.h for an explanation of this linkage
//
void ProcessInput1(bool Input1On) {

    if( SG3525Set.Input1.Print ) {
        StartMsg();
        PrintStringP(PSTR("Input1 "));
        if( Input1On ) PrintStringP(PSTR("ON\r\n" ));
        else           PrintStringP(PSTR("OFF\r\n"));
        }

    switch( SG3525Set.Input1.Action ) {

        //
        // Input is unused - just ignore
        //
        case INPUT_UNUSED:
            break;

        //
        // Direct xducer control - Turn xducer ON when button pressed
        //
        case INPUT_XCTRL:
            SG3525Run(Input1On);
            break;

        //
        // Push on/Push off - Toggle current state when button pressed
        //
        case INPUT_XPOPO:
            if( Input1On )
                SG3525Run(!SG3525_IS_ON);
            break;

        //
        // ESTOP when triggered
        //
        case INPUT_ESTOP:
            if( Input1On )
                SG3525Run(false);
            break;
        }
    }


void ProcessInput2(bool Input2On) {

    if( SG3525Set.Input1.Print ) {
        StartMsg();
        PrintStringP(PSTR("Input2 "));
        if( Input2On ) PrintStringP(PSTR("ON\r\n" ));
        else           PrintStringP(PSTR("OFF\r\n"));
        }

    switch( SG3525Set.Input2.Action ) {

        //
        // Input is unused - just ignore
        //
        case INPUT_UNUSED:
            break;

        //
        // Direct xducer control - Turn xducer ON when button pressed
        //
        case INPUT_XCTRL:
            SG3525Run(Input2On);
            break;

        //
        // Push on/Push off - Toggle current state when button pressed
        //
        case INPUT_XPOPO:
            if( Input2On )
                SG3525Run(!SG3525_IS_ON);
            break;

        //
        // ESTOP when triggered
        //
        case INPUT_ESTOP:
            if( Input2On )
                SG3525Run(false);
            break;
        }
    }


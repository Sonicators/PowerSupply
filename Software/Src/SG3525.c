//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
//      Copyright (C) 2015 Peter Walsh, Milford, NH 03055
//      All Rights Reserved under the MIT license as outlined below.
//
//  FILE
//      SG3525.c - Interface to the SG3525 SMPS controller
//
//  SYNOPSIS
//
//      See SG3525.h for details
//
//  DESCRIPTION
//
//      A simple driver module for the SG3525 digital potentiometer.
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

#include "SG3525.h"
#include "PWM.h"
#include "Freq.h"
#include "ACS712.h"
#include "Inputs.h"
#include "Outputs.h"
#include "SPIInline.h"
#include "Serial.h"

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// Data declarations
//
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

SG3525_SET  SG3525Set  NOINIT;
SG3525_CURR SG3525Curr NOINIT;

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// SG3525Init - Initialize the SG3525 interface
//
// Inputs:      None.
//
// Outputs:     None.
//
void SG3525Init(void) {

    SG3525_OFF;                                         // Set CS high to start
    _SET_BIT(_DDR(SG3525_CS_PORT),SG3525_CS_BIT);       // CS   is an output

    //
    // Initialize sub components to this module
    //
    SPIInit;
    PWMPotInit;
    FreqCPotInit;
    FreqFPotInit;
    FreqInit();
    PWMInit();
    ACS712Init();
    InputsInit();
    OutputsInit();
//    BuzzerInit();

    SG3525Set.Freq      = SG3525_DEF_FREQ;
    SG3525Set.Power     = SG3525_MIN_POWER;
    SG3525Set.RunMode   = RUN_CONTINUOUS;
    SG3525Set.RunTimer  = 0;

    SG3525Curr.RunTimer = 0;
    SG3525Curr.Freq     = 0;
    SG3525Curr.Current  = 0;
    SG3525Curr.Power    = 0;
    SG3525Curr.Vcc      = 0;
    SG3525Curr.Vc       = 0;
    SG3525Curr.PWM      = 0;

    SG3525Curr.PWMWiper   = 30;
    SG3525Curr.FreqCWiper = FreqCPot_MAX_WIPER/2+3;
    SG3525Curr.FreqFWiper = FreqFPot_MAX_WIPER/2;

    PWMPotSetWiper  (SG3525Curr.PWMWiper);
    FreqCPotSetWiper(SG3525Curr.FreqCWiper);
    FreqFPotSetWiper(SG3525Curr.FreqFWiper);
    }

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// SG3525Run - Enable/Disable SMPS output
//
// Inputs:      TRUE  if output should be enabled
//              FALSE otherwise
//
// Outputs:     None.
//
// NOTE: If SG3525Curr.RunMode == MODE_TIMED, will set timer and turn off output
//         when timer expires
//
void SG3525Run(bool Run) {

    if( Run ) {
        if( SG3525Set.RunMode == RUN_TIMED )
            SG3525Curr.RunTimer = SG3525Set.RunTimer;
        SG3525_ON;
        }
    else {
        SG3525Curr.RunTimer = 0;
        SG3525_OFF;
        }
    }


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// SG3525AdjustFreq - Station keeping for frequency setpoint
//
// Inputs:      None. Called periodically by the update program
//
// Outputs:     None.
//
#define DEFAULT_CAL     1000

void SG3525AdjustFreq(void) {
    bool    Changed = false;

#if 0
    if( SG3525_IS_ON ) {

        //
        // If we're ,amy steps away, take a big jump and reevaluate on the next
        //   time slice.
        //
        if( SG3525Curr.Freq+DEFAULT_CAL < SG3525Set.Freq ) {
            uint16_t FreqIncr = (SG3525Set.Freq - SG3525Curr.Freq + DEFAULT_CAL/2)/DEFAULT_CAL;
            SG3525Curr.FreqCWiper += FreqIncr;
            SG3525Curr.FreqFWiper = FreqFPot_MAX_WIPER/2;
            FreqCPotSetWiper(SG3525Curr.FreqCWiper);
            FreqFPotSetWiper(SG3525Curr.FreqFWiper);
            return;
            }


        if( SG3525Curr.Freq-DEFAULT_CAL > SG3525Set.Freq ) {
                uint16_t FreqIncr = (SG3525Curr.Freq - SG3525Set.Freq + DEFAULT_CAL/2)/DEFAULT_CAL;
            SG3525Curr.FreqCWiper -= FreqIncr;
            SG3525Curr.FreqFWiper = FreqFPot_MAX_WIPER/2;
            FreqCPotSetWiper(SG3525Curr.FreqCWiper);
            FreqFPotSetWiper(SG3525Curr.FreqFWiper);
            return;
            }
        }
#endif

    //
    // Adjust the frequency if needed.
    //
    if( SG3525Curr.Freq > SG3525Set.Freq ) {
        FreqFPotSetWiper(--SG3525Curr.FreqFWiper);
        Changed = true;
#       ifdef SHOW_TUNING
        PrintChar('-');
#       endif
        }

    if( SG3525Curr.Freq < SG3525Set.Freq ) {
        FreqFPotSetWiper(++SG3525Curr.FreqFWiper);
        Changed = true;
#       ifdef SHOW_TUNING
        PrintChar('+');
#       endif
        }

    if( !Changed )
        return;

    //
    // If we're approaching the limits of the fine-control pot, bump the coarse
    //   control and reset the fine.
    //
    if( SG3525Curr.FreqFWiper < 28 ) {
        SG3525Curr.FreqFWiper = FreqFPot_MAX_WIPER/2;
        SG3525Curr.FreqCWiper--;
        FreqFPotSetWiper(SG3525Curr.FreqFWiper);
        FreqCPotSetWiper(SG3525Curr.FreqCWiper);
#       ifdef SHOW_TUNING
        PrintChar('v');
#       endif
        }

    if( SG3525Curr.FreqFWiper > 228 ) {
        SG3525Curr.FreqFWiper = FreqFPot_MAX_WIPER/2;
        SG3525Curr.FreqCWiper++;
        FreqFPotSetWiper(SG3525Curr.FreqFWiper);
        FreqCPotSetWiper(SG3525Curr.FreqCWiper);
#       ifdef SHOW_TUNING
        PrintChar('^');
#       endif
        }
    }


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// SG3525AdjustPower - Station keeping for power setpoint
//
// Inputs:      None. Called periodically by the update program
//
// Outputs:     None.
//
#if 0
static void SG3525AdjustPower(void) {

    //
    // Adjust the power if needed.
    //
    if( SG3525Curr.Power > SG3525Set.Power ) {
        PWMPotSetWiper(--SG3525Curr.PWMWiper);
#       ifdef SHOW_PWR_TUNING
        PrintChar('<');
#       endif
        }

    if( SG3525Curr.Power < SG3525Set.Power ) {
        PWMPotSetWiper(++SG3525Curr.PWMWiper);
#       ifdef SHOW_PWR_TUNING
        PrintChar('>');
#       endif
        }
    }
#endif

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// SG3525Update - Update the generation system
//
// Inputs:      None
//
// Outputs:     None.
//
void SG3525Update(void) {

    //
    // Update all subordinate components
    //
    FreqUpdate();
    PWMUpdate();
    ACS712Update();
    InputsUpdate();

    //
    // If we're running, use the PWM version of frequency since it's the most accurate.
    //   Otherwise the PWM is offline so use the less-accurate count to keep us in the
    //   ballpark.
    //
    // Note: SG3525 counted output is twice the actual frequency
    //
    if( SG3525_IS_ON ) SG3525Curr.Freq = GetPWMFreq();
    else               SG3525Curr.Freq = GetFreq() >> 1;

    SG3525Curr.PWM     = GetPWM();
    SG3525Curr.Current = ACS712GetCurrent();
    SG3525Curr.Power   = SG3525Curr.Current*12;

    //
    // If we're running on timer, decrement and possibly stop
    //
    if( SG3525_IS_ON &&
        SG3525Set.RunMode == RUN_TIMED ) {
        if( --SG3525Curr.RunTimer == 0 )
            SG3525Run(false);
        }

    switch(SG3525Set.PwrMode) {

        //////////////////////////////////////////////////////////////////////////////////
        //
        // PWR_CONST_FREQ - Set constant frequency and power
        //
        case PWR_CONST_FREQ:
            SG3525AdjustFreq();
    //        SG3525AdjustPower();
            break;


        //////////////////////////////////////////////////////////////////////////////////
        //
        // PWR_CAL - Calibrate the digital pots
        //
        case PWR_CAL:
            SG3525Cal();
            break;


#ifdef USE_WIPER_CMDS
        //////////////////////////////////////////////////////////////////////////////////
        //
        // PWR_WIPER - User will use wiper commands to control system
        //
        case PWR_CONST_WIPER:
            break;
#endif


        default:        
            break;
        }

    }


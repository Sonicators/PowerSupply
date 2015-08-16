//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
//      Copyright (C) 2015 Peter Walsh, Milford, NH 03055
//      All Rights Reserved under the MIT license as outlined below.
//
//  FILE
//      SG3525Cal.c - Calibration mode for SG3525 circuit
//
//  SYNOPSIS
//
//  DESCRIPTION
//
//      Calibrate the digital pots
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
#include "SPIInline.h"
#include "Serial.h"
#include "MAScreen.h"

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// Data declarations
//
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

typedef enum {
    WAIT_START = 50,            // Waiting for ON command
    WAIT_28K,                   // Waiting for 28K frequency achieved
    UPPER_FREQ,                 // Measure upper frequency
    LOWER_FREQ,                 // Measure lower frequency
    END_CAL,                    // End calibration
    } SG3525_CAL_STEP;

static SG3525_CAL_STEP  CalStep;
static uint8_t          CalCount;

static uint16_t UpperFreq;
static uint16_t LowerFreq;

#define CAL_FREQ    28000       // Frequency at which calibration is done

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// SG3525Cal - Calibrate the SG3525 interface
//
// Inputs:      None.
//
// Outputs:     None.
//
void SG3525Cal(void) {

    //
    // We start in "OFF" mode, and begin calibration when the user turns the transducer
    //   "ON".
    //
    // If there's a problem, the user can turn off the transducer - interrupting the
    //   calibration. At that time they can then switch to a different mode, or restart
    //   the calibration by turning the transducer on again.
    //
    if( !SG3525_IS_ON ) {
        CalStep = WAIT_START;
        return;
        }

    //
    // Calibration running - Switch and process each step
    //
    switch(CalStep) {

        //
        // WAIT_START - We're running, so start the process
        //
        case WAIT_START:
            SG3525Curr.PWMWiper = 30;
            PWMPotSetWiper(SG3525Curr.PWMWiper);
            SG3525Set.Freq = CAL_FREQ;
            SG3525AdjustFreq();
            CalStep = WAIT_28K;
            break;

        //
        // WAIT_28K - Wait until we're close to 28KHz
        //
        case WAIT_28K:
            if( SG3525Curr.Freq < CAL_FREQ-10 ||
                SG3525Curr.Freq > CAL_FREQ+10 ) {
                SG3525AdjustFreq();
                return;
                }
            CalStep  = UPPER_FREQ;
            CalCount = 10;
            //
            // Fall through
            //      |
            //      V

        //
        // UPPER_FREQ - Take upper frequency measurement
        //
        case UPPER_FREQ:
            UpperFreq = SG3525Curr.Freq;
            CalStep   = LOWER_FREQ;
            FreqCPotSetWiper(--SG3525Curr.FreqCWiper);
            break;

        //
        // LOWER_FREQ - Take lower frequency measurement
        //
        case LOWER_FREQ:
            LowerFreq = SG3525Curr.Freq;
            CalStep  = UPPER_FREQ;
            FreqCPotSetWiper(--SG3525Curr.FreqCWiper);
PrintStringP(PSTR("Upper: "));
PrintD(UpperFreq,0);
PrintCRLF();
PrintStringP(PSTR("Lower: "));
PrintD(LowerFreq,0);
PrintCRLF();
PrintStringP(PSTR("Diff : "));
PrintD(UpperFreq-LowerFreq,0);
PrintCRLF();

            if( CalCount-- > 0 )
                break;

            //
            // Fall through
            //      |
            //      V

        //
        // END_CAL - End calibration mode
        //
        case END_CAL:
            SG3525Run(false);
            CalStep = WAIT_START;
            SG3525Set.PwrMode = PWR_CONST_FREQ;
            //
            // Scroll the data up so that it doesn't get garbled by the
            //   display update, and refresh the MA screen.
            //
            for( uint8_t i=0; i<20; i++ )
                PrintCRLF();
            ShowMAScreen();
            break;
        }

    }

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
//      Copyright (C) 2015 Peter Walsh, Milford, NH 03055
//      All Rights Reserved under the MIT license as outlined below.
//
//  FILE
//      Freq.h
//
//  SYNOPSIS
//
//
//      //////////////////////////////////////
//      //
//      // In Timer.h
//      //
//      ...Choose a timer                  (Default: Timer2)
//      ...Choose Polled or interrupt mode (Default: Interrupt)
//      ...Choose tick duration            (Default: 40ms)
//
//      //////////////////////////////////////
//      //
//      // In Freq.h
//      //
//      ...Choose a timer                  (Default: Timer0)
//
//      //////////////////////////////////////
//      //
//      // In Main.c
//      //
//      TimerInit();
//      FreqInit();                         // Called once at startup
//          :
//
//      while(1) {
//          while( !TimerUpdate() )
//              sleep_cpu();                // Wait for tick to happen
//
//          FreqUpdate();                   // Update frequency calculations
//          }
//
//  DESCRIPTION
//
//      Frequency processing
//
//      Calculate precision frequency using Timer0
//
//  EXAMPLE
//
//  VERSION:    2015.06.27
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

#ifndef FREQ_H
#define FREQ_H

#include <Timer.h>

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// This form of frequency measurement counts pulses on the T0 (Timer0) or T1 (Timer1)
//   inputs.
//
#define FREQ_TIMER_ID   0                       // Use TIMER0

//
// Uncomment FREQ_RISING_EDGES to count rising edges of the input signal, else system
//   will count FALLING edges
//
#define FREQ_RISING_EDGE

//
// End of user configurable options
//
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// FreqInit - Initialize frequency system
//
// Inputs:      None.
//
// Outputs:     None.
//
void FreqInit(void);


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// FreqUpdate - Update frequency measurement
//
// Inputs:      None.
//
// Outputs:     None.
//
void FreqUpdate(void);


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// GetFreq - Return currently measured frequency
//
// Inputs:      None.
//
// Outputs:     Measured frequency
//
uint16_t GetFreq(void);


#endif  // FREQ_H - entire file

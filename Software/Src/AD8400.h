//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
//      Copyright (C) 2015 Peter Walsh, Milford, NH 03055
//      All Rights Reserved under the MIT license as outlined below.
//
//  FILE
//      AD8400.h - SPI Interface to the AD8400 10K digital potentiometer
//
//  SYNOPSIS
//
//      //////////////////////////////////////
//      //
//      // In TestPot.h
//      //
//      #include <AD8400.h>
//
//      #define TestPot_PORT                    D
//      #define TestPot_BIT                     3
//      #define TestPot_MAXR                50000
//      #define TestPot_MAX_WIPER    AD8400_STEPS
//
//      #define TestPotInit             AD8400Init(TestPot_PORT,TestPot_BIT)
//      #define TestPotSetWiper(_w_)    AD8400SetWiper(TestPot_PORT,TestPot_BIT,_w_)
//      #define TestPotSetResist(_r_)   AD8400SetResist(TestPot_PORT,TestPot_BIT,TestPot_MAXR,_r_)
//      #define TestPotIncr             AD8400Incr(TestPot_PORT,TestPot_BIT)
//      #define TestPotDecr             AD8400Decr(TestPot_PORT,TestPot_BIT)
//
//      #define TestPotR2W(_r_)         AD8400_R2W(TestPot_MAXR,_r_)
//      #define TestPotW2R(_w_)         AD8400_W2R(TestPot_MAXR,_w_)
//
//      //////////////////////////////////////
//      //
//      // In Circuit
//      //
//      Connect AD8400 CS  (pin 3) to PORT/BIT specified in TestPot.h
//      Connect AD8400 SDI (pin 4) to PortB.3 (MOSI) of CPU
//      Connect AD8400 SCK (pin 5) to PortB.5 (SCK)  of CPU
//      
//      //////////////////////////////////////
//      //
//      // In main.c
//      //
//      TestPotInit();                      // Called once at startup
//
//      TestPotSetWiper(uint8_t);           // Set wiper value in steps
//      TestPotSetResist(uint8_t);          // Set wiper value in ohms
//      TestPotInc;                         // Increment wiper value
//      TestPotDec;                         // Decrement wiper value
//
//  DESCRIPTION
//
//      A simple driver module for the AD8400 digital potentiometer.
//
//  VERSION:    2015.05.18
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

#ifndef AD8400_H
#define AD8400_H

#include <stdint.h>

#include "SPIInline.h"

#define AD8400_STEPS        256

#define AD8400_R2W(_M_,_R_)         ((uint16_t) SCALE_RATIO(((uint32_t) (_R_)),AD8400_STEPS,_M_))
#define AD8400_W2R(_M_,_W_)         ((uint16_t) SCALE_RATIO(((uint32_t) (_W_)),_M_,AD8400_STEPS))

#define AD8400_SELECT(_p_,_b_)      _CLR_BIT(_PORT(_p_),_b_)
#define AD8400_DESELECT(_p_,_b_)    _SET_BIT(_PORT(_p_),_b_)

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// AD8400Init - Initialize the AD8400 interface
//
// Inputs:      None.
//
// Outputs:     None.
//
#define AD8400Init(_p_,_b_) {                                                           \
    AD8400_DESELECT(_p_,_b_);                           /* Set CS high to start */      \
    _SET_BIT(_DDR(_p_),_b_);                            /* CS   is an output    */      \
    }                                                                                   \


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// AD8400SetWiper - Set wiper to specified value
//
// Inputs:      Wiper value to set (0 .. 255)
//
// Outputs:     None.
//
#define AD8400SetWiper(_p_,_b_,_W_) {                                                   \
    AD8400_SELECT(_p_,_b_);                                                             \
    PutSPIByte(0);                                                                      \
    PutSPIByte(_W_);                                                                    \
    AD8400_DESELECT(_p_,_b_);                                                           \
    }                                                                                   \


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// AD8400SetResist - Set resistance value
//
// Inputs:      Resistance (0 .. 1000)
//
// Outputs:     None.
//
#define AD8400SetResist(_p_,_b_,_MAXR_,_R_) {                                           \
    AD8400SetWiper(_p_,_b_,AD8400_R2W(_MAXR_,_R_))                                      \
    }                                                                                   \


#endif  // AD8400_H - entire file

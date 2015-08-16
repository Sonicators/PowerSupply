//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
//      Copyright (C) 2015 Peter Walsh, Milford, NH 03055
//      All Rights Reserved under the MIT license as outlined below.
//
//  FILE
//      Freq.c
//
//  DESCRIPTION
//
//      Frequency processing
//
//      Setup a timer as a real-time frequency counter
//
//      See Freq.h for an in-depth description
//
//  VERSION:    2010.12.19
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

#include <string.h>

#include <avr\io.h>
#include <avr\interrupt.h>

#include <Freq.h>
#include <TimerMacros.h>

#include <Debug.h>

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// Data declarations
//
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static struct {
    uint16_t    Counts[TICKS_PER_SEC];              // Counter values for last 1 second
    uint16_t    *Ptr;                               // Index to next place to store values
    uint16_t    PrevTimer;                          // Previous extended timer
    uint8_t     TimerExt;                           // Extended timer count
    } Freq NOINIT;

//////////////////////////////////////////////////////////////////////////////////////////
//
// Setup some port designations
//
#define PRTIMx      _PRTIM(FREQ_TIMER_ID)
#define FREQ_ISR    _TOVF_VECT(FREQ_TIMER_ID)

#define TCCRAx      _TCCRA(FREQ_TIMER_ID)
#define TCCRBx      _TCCRB(FREQ_TIMER_ID)
#define TIMSKx      _TIMSK(FREQ_TIMER_ID)
#define TCNTx       _TCNT(FREQ_TIMER_ID)
#define TOIEx       _TOIE(FREQ_TIMER_ID)

#define FREQ_MODE   0                               // Normal counting mode

#ifdef FREQ_RISING_EDGE
#define FREQ_INPUT  _PIN_MASK(_CS2(FREQ_TIMER_ID)) | \
                    _PIN_MASK(_CS1(FREQ_TIMER_ID)) | \
                    _PIN_MASK(_CS0(FREQ_TIMER_ID))
#else
#define FREQ_INPUT  _PIN_MASK(_CS2(FREQ_TIMER_ID)) | \
                    _PIN_MASK(_CS1(FREQ_TIMER_ID))
#endif

#define DISABLE_INT { TIMSKx = 0; }                 // Disable timer interrupts
#define ENABLE_INT  { TIMSKx = _PIN_MASK(TOIEx); }  // Allow interrupts


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// FreqInit - Initialize timer system
//
// Inputs:      None.
//
// Outputs:     None.
//
void FreqInit(void) {

    memset(&Freq,0,sizeof(Freq));

    Freq.Ptr = Freq.Counts;

    _CLR_BIT(PRR,PRTIMx);           // Powerup the clock

    //
    // Setup the timer as free running counter
    //
    TCCRAx = FREQ_MODE;             // No output compare functions
    TCCRBx = FREQ_INPUT;            // Set input = T0
    TCNTx  = 0;

    ENABLE_INT;                     // Allow interrupts
    }

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// FreqUpdate - Update frequency
//
// Inputs:      None.
//
// Outputs:     None.
//
void FreqUpdate(void) {

    //
    // Standard interrupt procedures apply: watch out for timer updating
    //   and overflowing in between getting the two values.
    //
    uint8_t TimerCopy;
    uint8_t ExtCopy;

    DISABLE_INT;                         // Disable timer interrupts

    do {
        TimerCopy = TCNTx;
        ExtCopy   = Freq.TimerExt;
        } while( TimerCopy != TCNTx );

    ENABLE_INT;

    uint16_t CurrTimer = (ExtCopy << 8) + TimerCopy;

    *Freq.Ptr++     = CurrTimer-Freq.PrevTimer;
     Freq.PrevTimer = CurrTimer;

    //
    // Roll over the pointer after reaching the end
    //
    if( Freq.Ptr >= Freq.Counts + NUMOF(Freq.Counts) )
        Freq.Ptr  = Freq.Counts;
    }

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// GetFreq - Return currently measured frequency
//
// Inputs:      None.
//
// Outputs:     Measured frequency over the last second
//
uint16_t GetFreq(void) { 
    uint16_t Result = 0;

    for( uint8_t i=0; i<NUMOF(Freq.Counts); i++ )
        Result += Freq.Counts[i];

    return Result;
    }


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// _TOVF_vect - Overflow timer count
//
// Just increment the extended byte, making an equivalent 16-bit timer
//
// Inputs:      None. (ISR)
//
// Outputs:     None.
//
ISR(FREQ_ISR,ISR_NOBLOCK) { Freq.TimerExt++; }

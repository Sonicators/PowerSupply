//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
//      Copyright (C) 2015 Peter Walsh, Milford, NH 03055
//      All Rights Reserved under the MIT license as outlined below.
//
//  FILE
//      SG3525.h - Interface to the SG3525 SMPS controller
//
//  SYNOPSIS
//
//      //////////////////////////////////////
//      //
//      // In SG3525.h
//      //
//      ...Set chip select port and bit    (Default: PortD.7)
//
//      //////////////////////////////////////
//      //
//      // In main.c
//      //
//      SG3525Init();                       // Called once at startup
//
//      SG3525Run(bool);                    // Turn device on/off
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

#ifndef SG3525_H
#define SG3525_H

#include <stdbool.h>

#include "PortMacros.h"

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// The driver will pull this CS line low while communicating with the chip.
//
#define SG3525_CS_PORT                          D       // Chip select: Port D
#define SG3525_CS_BIT                           7       // Chip select: Control pin

//
// PWMPot - 10K ohm MCP4161 on PortD.2
//
#define PWMPot_PORT                             D
#define PWMPot_BIT                              2
#define PWMPot_MAXR                         10000
#define PWMPot_MAX_WIPER        (MCP4161_STEPS-1)

//
// FreqCPot - 50K MCP4131 on PortD.3 (Coarse frequency adj.)
//
#define FreqCPot_PORT                           D
#define FreqCPot_BIT                            3
#define FreqCPot_MAXR                       50000
#define FreqCPot_MAX_WIPER      (MCP4131_STEPS-1)

//
// FreqFPot - 1K AD8400 on PortD.6 (Fine frequency adj.)
//
#define FreqFPot_PORT                           D
#define FreqFPot_BIT                            6
#define FreqFPot_MAXR                        1000
#define FreqFPot_MAX_WIPER       (AD8400_STEPS-1)

//
// Uncomment this to disable frequency tracking and allow direct wiper setting
//   commands
//
#define USE_WIPER_CMDS

//
// Uncomment this to disable frequency/power adjustment commands
//
#define USE_ADJ_CMDS

//
// Uncomment this to print single-chars that show the frequency tuning
//
//#define SHOW_TUNING

//
// Uncomment this to print single-chars that show the power tuning
//
#define SHOW_PWR_TUNING


//
// End of user configurable options
//
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

//
// Project parameters
//
#define SG3525_MIN_FREQ     20000       // Minimum frequency we allow
#define SG3525_DEF_FREQ     28000       // Default frequency at startup
#define SG3525_MAX_FREQ     35000       // Maximum frequency we allow

#define SG3525_MIN_POWER    0           // Minimum power we allow
#define SG3525_MAX_POWER    (100*10)    // Maximum power we allow (in watts x 10)

//
// Convenience macros
//
#define SG3525_ON           _CLR_BIT(_PORT(SG3525_CS_PORT),SG3525_CS_BIT)
#define SG3525_OFF          _SET_BIT(_PORT(SG3525_CS_PORT),SG3525_CS_BIT)
#define SG3525_IS_ON        _BIT_OFF(_PORT(SG3525_CS_PORT),SG3525_CS_BIT)

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// RunMode
//
typedef enum {
    RUN_CONTINUOUS = 100,           // On stays on
    RUN_TIMED,                      // Timed run
    } SG3525_RUN_MODE;

#define NUM_RUN_MODES   ( RUN_TIMED - RUN_CONTINUOUS + 1 )

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// PowerMode
//
typedef enum {
    PWR_CONST_FREQ = 200,           // Constant freq and power
    PWR_CAL,                        // Calibration mode
#ifdef USE_WIPER_CMDS
    PWR_CONST_WIPER,                // User debug via wiper commands
#endif
    } SG3525_PWR_MODE;

#define NUM_PWR_MODES     ( PWR_CONST_WIPER - PWR_CONST_FREQ + 1 )
#define IDX_PWR_MODE(_x_) (_x_ - PWR_CONST_FREQ)        // Index of 1st power mode

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// Input information
//
//      Action
//      MaxPower    Maximum power this transducer can sustain (watts)
//
typedef enum {
    INPUT_UNUSED = 300,     // Input is unused
    INPUT_XCTRL,            // Xducer control: on while pushed
    INPUT_XPOPO,            // Xducer control: Push on/push off
    INPUT_ESTOP,            // ESTOP when triggered
    } INPUT_ACTION;

#define NUM_ACTIONS     ( INPUT_ESTOP - INPUT_UNUSED + 1 )
#define IDX_ACTION(_x_) (_x_ - INPUT_UNUSED)            // Index of 1st input action

typedef struct {
    INPUT_ACTION    Action; // What to do when activated
    bool            Print;  // TRUE if should print msg when activated
    } INPUT;


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// SG3525Set - Parameters set by the user, SG3525 module will try to keep these settings
//
typedef struct {
    uint16_t        Freq;           // Requested target frequency (Hz)
    uint16_t        Power;          // Requested target power     (watts x 10)

    SG3525_RUN_MODE RunMode;        // Current mode, when running
    uint16_t        RunTimer;       // Countdown timer, when in RUN_TIMED mode

    SG3525_PWR_MODE PwrMode;        // Power output mode

    INPUT           Input1;         // Input actions
    INPUT           Input2;
    } SG3525_SET;

extern SG3525_SET SG3525Set;

//////////////////////////////////////////////////////////////////////////////////////////
//
// SG3525Curr - Actual current parameters, measured by the SG3525 module
//
// NOTE: OF and OFF indications come directly from the CS output macro
//
typedef struct {
    uint16_t    RunTimer;   // Countdown timer, when in RUN_TIMED mode

    uint16_t    Freq;       // Current frequency
    uint16_t    Current;    // Current current
    uint16_t    Power;      // Transducer power, in watts x 10

    uint16_t    Vcc;        // Vcc, in volts x 10
    uint16_t    Vc;         // Vc , in volts x 10
    uint16_t    PWM;        // PWM, in     % x 10

    uint16_t    PWMWiper;   // Current PWM         wiper
    uint16_t    FreqCWiper; // Current coarse freq wiper
    uint16_t    FreqFWiper; // Current fine   freq wiper
    } SG3525_CURR;

extern SG3525_CURR SG3525Curr;

//
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// PWMPot setup
//
#include "MCP4161.h"

#define PWMPotInit              MCP4161Init(PWMPot_PORT,PWMPot_BIT)
#define PWMPotSetWiper(_w_)     MCP4161SetWiper(PWMPot_PORT,PWMPot_BIT,_w_)
#define PWMPotSetResist(_r_)    MCP4161SetResist(PWMPot_PORT,PWMPot_BIT,PWMPot_MAXR,_r_)
#define PWMPotIncr              MCP4161Incr(PWMPot_PORT,PWMPot_BIT)
#define PWMPotDecr              MCP4161Decr(PWMPot_PORT,PWMPot_BIT)

#define PWMPotR2W(_r_)          MCP4161_R2W(PWMPot_MAXR,_r_)
#define PWMPotW2R(_w_)          MCP4161_W2R(PWMPot_MAXR,_w_)


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// FreqCPot setup
//
#include "MCP4131.h"

#define FreqCPotInit            MCP4131Init(FreqCPot_PORT,FreqCPot_BIT)
#define FreqCPotSetWiper(_w_)   MCP4131SetWiper(FreqCPot_PORT,FreqCPot_BIT,_w_)
#define FreqCPotSetResist(_r_)  MCP4131SetResist(FreqCPot_PORT,FreqCPot_BIT,FreqCPot_MAXR,_r_)
#define FreqCPotIncr            MCP4131Incr(FreqCPot_PORT,FreqCPot_BIT)
#define FreqCPotDecr            MCP4131Decr(FreqCPot_PORT,FreqCPot_BIT)

#define FreqCPotR2W(_r_)        MCP4131_R2W(FreqCPot_MAXR,_r_)
#define FreqCPotW2R(_w_)        MCP4131_W2R(FreqCPot_MAXR,_w_)


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// FreqFPot setup
//
#include "AD8400.h"

#define FreqFPotInit            AD8400Init(FreqFPot_PORT,FreqFPot_BIT)
#define FreqFPotSetWiper(_w_)   AD8400SetWiper(FreqFPot_PORT,FreqFPot_BIT,_w_)
#define FreqFPotSetResist(_r_)  AD8400SetResist(FreqFPot_PORT,FreqFPot_BIT,FreqFPot_MAXR,_r_)
#define FreqFPotIncr            AD8400Incr(FreqFPot_PORT,FreqFPot_BIT)
#define FreqFPotDecr            AD8400Decr(FreqFPot_PORT,FreqFPot_BIT)

#define FreqFPotR2W(_r_)        AD8400_R2W(FreqFPot_MAXR,_r_)
#define FreqFPotW2R(_w_)        AD8400_W2R(FreqFPot_MAXR,_w_)


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// SG3525Init - Initialize the SG3525 interface
//
// Inputs:      None.
//
// Outputs:     None.
//
void SG3525Init(void);
                                                                                        \

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// SG3525Run - Enable/Disable output
//
// Inputs:      TRUE  if output should be enabled
//              FALSE otherwise
//
// Outputs:     None.
//
// NOTE: If SG3525Curr.RunMode == MODE_TIMED, will set timer and turn off output
//         when timer expires
//
void SG3525Run(bool Run);


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// SG3525Update - Update the generation system
//
// Inputs:      None
//
// Outputs:     None.
//
void SG3525Update(void);


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// SG3525AdjustFreq - Station keeping for frequency setpoint
//
// Inputs:      None. Called periodically by the update program
//
// Outputs:     None.
//
void SG3525AdjustFreq(void);


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// SG3525Cmd - Manage typed commands aimed at the SG3525 system
//
// Inputs:      None.
//
// Outputs:     TRUE  if we understood and processed command
//              FALSE if command isn't ours, belongs to another subsystem
//
bool SG3525Cmd(char *Command);


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//
// SG3525Cal - System calibration
//
// Inputs:      None.
//
// Outputs:     None.
//
void SG3525Cal(void);


#endif  // SG3525_H - entire file

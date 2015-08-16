Arduino Nano contolled ultrasonic power driver board.

This is created/edited with KiCad. Just open the project files, it should work.

Lib/*                       Component specific to this circuit
Schematic.sch               Top level schematic, consisting of 4 sub pages
    Microcontroller.sch         Microcontroller connections
    Switcher.sch                SG3525 switcher system
    PowerDriver.sch             Half-bridge power driver
    Connectors.sch              Board connections and jumpers
    
//////////////////////////////////////////////////////////////////////////////////////////
// 
// *** WARNING ***
// 
// This schematicdoesn't track the software.
// 
// This is version 1.2 of the circuit, which is incompatible with version 1.0 of
//   the software.
// 
// While doing the schematic and board layout, I made some minor changes with the 
//   intention of updating the software once the PCBs came in.
// 
// I then discovered that the existing system has problems, and a new circuit V2.0 is
//   being built, which will require V2.0 of the software.
// 
// So software is V1.0, while schematic is V1.2, and both are obsolete.
//
// If you want to make the V1.2 board and use this software, contact me for details,
//   you'll need some minor changes.
// 
//////////////////////////////////////////////////////////////////////////////////////////

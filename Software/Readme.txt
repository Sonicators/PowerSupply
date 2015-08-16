Arduino Nano software that runs the power supply board.

This is controller program code, *not* a sketch!

This software must be burned into the program memory of the Arduino microprocessor; for example, by using an AVR Dragon through the ICSP port. As such it will replace the existing Arduino bootloader... so make a backup if you think you might want to use the bootloader in the future.

This software compiles under AVR Studio 4.18 and WinAVR: open the project file and select "Build All".

To use a different toolchain, make a new project using your software, import all the files in the Src directory and compile. (You will need WinAVR system include files.)

./Sone.*			AVR Studio Project files
Default/Sone.hex		Precompiled hex file, ready for programming
Src/*				All the source files

To run this software:

Plug in the board and use the device manager to determine the serial port. (On my system, I see COM1 and COM7... COM1 is on the motherboard, and COM7 is the new port added when the Arduino is plugged in.)

Open a serial terminal window (Hyperterm or Putty serial or equivalent) 19200,8,N,1 to that port.

The software should display a status screen using VT100 commands.

See MAScreen.c, Setup.c, and SG3525Cmd.c for commands and usages.

//////////////////////////////////////////////////////////////////////////////////////////
// 
// *** WARNING ***
// 
// This software doesn't track the schematic.
// 
// This is version 1.0 of the software, which works with version 1.0 of the power
//   driver board.
// 
// While doing the schematic and board layout, I made some minor changes with the 
//   intention of updating the software once the PCBs came in. The schematic is
//   therefore version 1.2.
// 
// I then discovered that the existing system has problems, and a new circuit V2.0 is
//   being built, which will require V2.0 of the software.
// 
// So this software is V1.0, while the schematic is V1.2, and both are obsolete.
//
// If you want to make the V1.2 board and use this software, contact me for details,
//   you'll need some minor changes.
// 
//////////////////////////////////////////////////////////////////////////////////////////

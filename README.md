# Context
This is an INSA Lyon - 3TCA SON project.
We must process audio signal using a Teensy 4.0 Printed Circuit Board.

# Description
This project uses a Guitar Hero Wii Les Paul controller as input to an electric guitar sound synthesizer using a Teensy PCB.
Wii NES Nunchuck connectors being very old and poorly documented, we had to find out what each cable meant and how to connect them, leading to the following layout:

- Green - 3.3V
- White - SCL (19)
- Black - X
- Yellow - SDA (18)
- Red - GND

These pins are connected to a breadboard and then connected to the Teensy, to whom the guitar communicates using I²C. Inputs from the guitar can then be read and used as parameters to an electric guitar synthesizer.
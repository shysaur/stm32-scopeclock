# STM32 Oscilloscope Clock

This project implements a simple clock exploiting the XY input of an
oscilloscope. Many other similar projects exist, this one's peculiarity is
that it uses an STM32 microcontroller -- specifically an STM32F100.
I expect the code to be easily adaptable to other higher performing STM32 chips
(as the STM32F100 is the least performing of them all and the same hardware
is available with a few modifications on the entire line).

The X-Y output is on pins PC4 and PC5 (the DAC is hardwired there so there is
not much choice). An external 8 MHz clock is also expected.

At the moment there are no features (not even setting the clock) so this
program is just for demonstration. Other features will be implemented in the
future (time permitting).

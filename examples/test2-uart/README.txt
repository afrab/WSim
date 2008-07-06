uart_test
=========

Overview
--------
This is a simple hardware UART test program. It receives text lines over
the serial port and writes back a status msg with length and contents of
the buffer.

Default settings for the serial line are 9600 Baud, 8 data and 1 bits,
no parity.

Features are:

Taskhandler/Eventhandler based design. Most of the time it stays in LPM0,
while UART receive interrupts are handled. When a newline '\n' character
is received, it seta an event/task flag and wakes up the main loop.
The example task just writes out the received buffer.

Received characters are visible on P1OUT if a LED array is connected.


Required hardware
-----------------

- MSP430F135 or larger device with hardware UART (uses XT2 can be adapted
  to F123 or similar if basic clock settings are canged in hardware.h)

- Level shifter / RS232 interface on port 3, USART0, TX: P3.4, RX: P3.5

- 8 MHz crystal on XT2 (other crystals possible, but the baudrate
  has to be adjusted in hardware.h)


Source
------
Files_:

- README.txt_           This file
- asmlib.h_             Header file for asmlib.h
- asmlib.S_             Timer_A and UART interrupts
- hardware.h_           Pins and port descriptions
- main.c_               Main application
- makefile_             Build rules
- serialComm.c_         Simple serial command line implementation
- serialComm.h_         Header file for serialComm.c
- taskhandler.h_        Header file for taskhandler.h
- taskhandler.S_        Eventhandler
- tasklist.h_           Event list for the Eventhandler

.. _files: .
.. _README.txt: README.txt
.. _asmlib.h: asmlib.h
.. _asmlib.S: asmlib.S
.. _hardware.h: hardware.h
.. _main.c: main.c
.. _makefile: makefile
.. _serialComm.c: serialComm.c
.. _serialComm.h: serialComm.h
.. _taskhandler.h: taskhandler.h
.. _taskhandler.S: taskhandler.S
.. _tasklist.h: tasklist.h


Disclaimer
----------
This example is part of the mspgcc project http://mspgcc.sf.net.
See license.txt_ for details.


References
----------
- http://mspgcc.sf.net: GNU C for the MSP430
- TI MSP430 home: http://www.ti.com/msp430


Chris Liechti <cliechti@gmx.net>


------

back_

.. _back: ../readme.html
.. _license.txt: ../license.html


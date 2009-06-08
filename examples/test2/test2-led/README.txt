leds
====

Overview
--------
It is a simple example project for the MSP430 series MCU and the GCC port
of the mspgcc project. The project contains a makefile and C sources.

Features:

- Just a plain ``main()`` that accesses the ports of the MSP

- Makefile

  - compile and link
  - convert to intel hex format
  - generate a listing with mixed C / assembly


Required hardware
-----------------

- A MSP430F1121 or larger device (any from the F1x series)

- An array of eight LEDs on P1.0 .. P1.7 (470 Ohms series resistor to GND)
  or better connected through a driver IC like a 74HC245 or 74HC244.


Source
------
Files_:

- README.txt_   This file
- hardware.h_   Pins and port descriptions
- main.c_       Main application
- makefile_     Build rules

.. _files: .
.. _README.txt: README.txt
.. _hardware.h: hardware.h
.. _main.c: main.c
.. _makefile: makefile



Disclaimer
----------
This example is part of the mspgcc project http://mspgcc.sf.net.
See license.txt_ for details.

chris

------

back_

.. _license.txt: ../license.html
.. _back:       ../readme.html

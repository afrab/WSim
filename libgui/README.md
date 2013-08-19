WSim GUI Library
================
You need Qt 4.6+ to build the WSim Gui.

Maintainer: David Gräff <david.graeff@web.de>
For bug reports please use the issues function of github at
https://github.com/davidgraeff/WSim/issues.

This readme serves as how-to and brief documentation
for device developers.

Overview
--------
Because the Qt window need to run in the main thread, we
cannot use the main() method in src/main.c for starting
the application. Instead main() in libgui/qt_main.cpp
is used. The simulations main() is started by the SimulationThread
class in another thread. This allows us to stop/restart a
simulation and let the gui survive a SIGBUS/SIGILL within
the simulation thread. Qt also forces us to use C++ for
compiling the libgui, if you want to interact with the
rest of WSim (plain C) keep in mind to use __"extern C"__
statements for including other header files.

The user interface is quiet simple and self explaining,
but supports some common hardware inputs and outputs for
device developers. Details are explained in the following
sections.

You may easily change the interface to your needs for
educational purposes for example by using the Qt Designer
and open _mainwindow.ui_. For advanced extensions
you may need to hack the _MainWindow class_.

To make the gui an optional part, the only interaction
between gui and the simulation is in the ui.cpp file which
knows the SimulationThread object. SimulationThread is a
global object and takes care of thread synchronisations.

Simulated graphical platform interface
--------------------------------------
The main and centered part of the WSIm GUI is for drawing
a representation of the hardware output interface (like an
lcd or a segment display). FOr this to work you need to provide us a
memory pointer, memory size, pixmap width and height,
where the data is organized in rows with a pixel
represented by 3 bytes (red, green, blue intensity) each.

How to add buttons to the gui for my platform/device?
-----------------------------------------------------
To represent hardware buttons on the simulator gui,
you can use a generic way for adding buttons.
You may define the order of appearence and the text
on the buttons. For further control, you have to
access the _MainWindow class_ (your device driver have to
be compiled with a c++ compiler as a result).

An example for two buttons, defined in your_platform_create():
    uint8_t index=0;
    machine.ui.buttons[index].pin = UI_BUTTON_3;
    strncpy(machine.ui.buttons[index++].name, "Arrow up", 255);

    machine.ui.buttons[index].pin = UI_BUTTON_5;
    strncpy(machine.ui.buttons[index++].name, "Arrow down", 255);
Two properties are necessary: The text of the button, in
this example we have "Arrow up" and "Arrow down" respectivly.
The second property is the internal id or pin of the button.
You may define up to 8 buttons and use UI_BUTTON_1..8 for the id.

For interpretation of the button states please refer to the wsim
documentation or study the ez430chronos plaform code in
platform/ez430chronos/ez430chronos.c at "devices_update()".


How to add leds to the gui for my platform/device?
--------------------------------------------------
For now, you have to draw your leds to the hardware-representation-pixmap.

How to add any other output elements to the gui for my platform/device?
-----------------------------------------------------------------------
For now, you have to draw your controls to the hardware-representation-pixmap.
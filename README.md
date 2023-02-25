The Virtual Micral N
====================

This program in a low level emulator dedicated to the study and presentation
of the historical microcomputer *Micral N*.


Status
------

The emulator currently emulates some cards of the Micral N.
As the emulator is a way to conduct research on the inner working of the computer,
some parts are « guesses » on how signals are constructed and propagated.

When documentation is found and analysis of hardware is done, the emulation is fixed.

Executables
-----------

There are two ways of launching the emulator. One is a command line tool which is aimed
at running a preconfigured Micral N. Another is a GUI tool to graphically play around
and discover how the computer works.

The emulator consists of several libraries with a dedicated objective to allow other
executables to be built. For example, it could serve on a hardware panel reconstruction,
or a graphical one, without all the debug tools. Even pushing it to a guided tour.

A small disassembler with support for Micral N dedicated instructions is also available.

Configuration
-------------

At the moment, configuration if by changing the source only. In the `main.cpp` file,
the enum passed to the `Simulator` constructor creates a particular scenario. By changing
the value, the configuration changes.

Future work needs to be done for data driven configuration.

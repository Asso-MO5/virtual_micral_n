The Virtual Micral N
====================

This program in a low level emulator dedicated to the study and presentation
of the historical micro computer *Micral N*.


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

Other projected use are an assembler and a disassembler.

We could think about a WASM target for easier access to everyone.

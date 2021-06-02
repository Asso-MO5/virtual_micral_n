# Micral N Emulation notes

The main objective of this emulator is to be a platform to study the Micral N system.

Consequently, the choice was made for its architecture to be around a Low Level Emulation, at least in the exchanges
between the cards, through the Pluribus.

The main tool for synchronisation is the *Owned Data*. There are two kinds of Signals: the
*OwnedSignal* and the *OwnedValue*.

The emulation of the CPU is at memory cycle level, thus allowing to generate the synchronisation signals with
a good granularity.

Emulation devices can react with two different manner to a signal. They can react immediately, changing a state
or sending a signal. They can also ask to be scheduled for a future reaction to a received signal. The latter
way is slower and more performance consuming, but allows playing with the device delays.

## Global architecture of the project

The project is composed of different sub-projects:

Executables:

  * `cli` is a program aimed at launching an emulation from the command line,
  * `gui` is a program aimed at launching a full study environment with graphical feedbacks.

Libraries:

  * `devices` implements the devices used by the Micral N: the cards, the Pluribus, and some
components on the cards, like the CPU or the Clock.
  * `mcs8` implements some tools specific to the 8008 micro-processor.
  * `emulation_core` implements the core of the emulator, with the scheduling and synchronisation system.
  * `emulator` implements the emulator itself, which ties together the `devices` and puts into use
    the `emulation_core` bricks to run the emulated system. 

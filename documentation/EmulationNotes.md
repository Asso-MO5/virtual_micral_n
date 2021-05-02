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

## Signals

All signals in the simulation are asserted *logically* (HIGH means asserted) rather than *physically* (which depends on
the signal implementations). For example, ```READY/``` and ```INTERRUPT/``` are physically asserted LOW for the 8008,
but in the simulation, they are asserted HIGH.

The reason is to keep the logic intuitive at programming level without having to care about which actual assertion is
physically right.

To have correct logging or graphical user feedback, it is thus necessary to correct the value.

### Sub note

This could be taken care of by some automatic machinery in the future, by differentiating the signal value and the
assertion.


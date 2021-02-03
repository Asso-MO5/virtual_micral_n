## Signals

All signals in the simulation are asserted *logically* (HIGH means asserted) rather than *physically* (which depends on
the signal implementations).

For example, ```READY/``` and ```INTERRUPT/``` are physically asserted LOW for the 8008, but in the simulation, they are
asserted HIGH.

The reason is to keep the logic intuitive at programming level without having to care about which actual assertion is
physically right.

To have correct logging or graphical user feedback, it is thus necessary to correct the value.

### Sub note

This could be taken care of by some automatic machinery in the future, by differentiating the signal value and the
assertion.


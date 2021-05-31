Emulation Core
==============

As the name says, this library is about the tools that form the core of the emulation
system.

The base duo of the emulation core is formed by the [`Scheduler`](src/Scheduler.h)
and its [`Schedulable`](src/Schedulable.h)s.

The Schedulables communicate with the help of [`State`](src/State.h) and [`Edge`](src/Edge.h).

A `State` represents a logical electronic state in the system and can be either `HIGH` or
`LOW`.

An `Edge` represents a `State` value transition, with the moment it happens.

A moment, for the system, is represented by a [`Scheduling::counter_type`](src/Scheduling.h).
This type must be used for any representation of time. For evolution purposes, the
underlying type must not be used.

A specific time value is obtained by [`Scheduling::unscheduled()`](src/Scheduling.h). It
represents a time that the `Scheduler` will never reach. As a result, a `Schedulable`
whose next even is scheduled at this pseudo-time will never be called by the
`Scheduler`.

A `Schedulable` can be activated by a callback attached to an `OwnedValue` or a `OwnedSignal` though.

An [`OwnedSignal`](src/OwnedSignal.h) is a facility which holds a `State` and will
broadcast to any subscriber the `Edge` created by its `State` change. To be able
to change the value of an `OwnedSignal`, a request for control must be done before.
Only part of the program holding the ownership can change the value of an `OwnedSignal`.

The key to control the signal (generally `this`) must not be shared with other objects.
It would defeat the whole ownership mechanism.

An [`OwnedValue`](src/OwnedValue.h) shares the same principles of ownership and broadcasting
to subscribers as the `OwnedSignal`. An `OwnedValue` holds a value. In the context
of the emulation, this value will be a `std::uint8_t` or `std::uint16_t`, depending
on the bus width it represents.

Lastly, [`Frequency`](src/Frequency.h) is a class with a user defined literal `_hz`
to manipulate frequency.

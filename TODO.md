### Bug?

* While using the ROM Programming Tape command in the monitor, hex values of B are
  replaced by '*'. Check why.

### Implementation

* Implement the Serial card
  * Create a Hello World for the TTY
  * Attach the TTY the CLI executable.
* Implement the possibility to receive and send punch data through the Serial Card
* Add a script system to manipulate the simulation
  * Can read from a console
  * Can read from a file
  * Commands can be:
    * (g)o (for simulator)
    * (p)ause (for simulator)
    * (s)et switch / data
    * (w)ait for status
* Panel and Console Card implementation
  * Implement the AV switch (Need to understand better)
  * Implement both back panel switches
* Implement the ROM/RAM Switching on the 4k RAM/ROM card.
  * Improve debugging/gui
    * add a callback telling what data changed.
    * thus, it's the responsibility of the debug/gui to update its view when it changes, rather than
      reconstructing it each frame.
* Implement the Stack card
  * Channel Mode (still needs to be understood)
* Implement the Disk Controller
  * Implement delays (on reading disk)
* Implement the I/O cards
  * Implement the Interrupts for the I/O cards
* Implement a possibility to connect OwnedSignals to a concentrated OwnedValue (for IOCard/SerialCard)
  * At the moment, the Serial Card "cheats" by hardcoding the signals in a status word
* Add an Instruction Factory returning coded bytes, used by the Interrupt System opcode jamming (will also be useful for
  an assembler)
* Add an assembler project with an objective of compiling SCELBAL
* Implement a possibility to subscribe to a specific bit on an OwnedValue? (where was it needed?)
* Change LED colors to correctly matching the hardware.
* Create a debug panel for the DiskController
  * Show write status when implemented.
* DiskReader can be a Schedulable to emulate the reading/writing times

### Architecture

* MemoryViewer system to verify. It's weird to have it in i8008 library
* PluribusHelper, shouldn't it be i8008 Helper ?

### Improvements

* Add Architecture.md in documentation/ explaining and linking to local library documentation
  and with a dependency diagram (generated from cmake).
* Add a check that parse every README.md from libraries and check that names in backquotes
  are actual names found in the code. Also check that every class name in the code is
  referenced at least once in a (local?) README.md. It could also produce a cross-reference file.
* Remove the cppcheck warnings
* Most of the signals react on a specific edge. Add possibility to subscribe to a specific edge
* Add a clang-tidy configuration
* Sets cppcheck and clang-tidy to a CI
* Use the LED remanence system on the EXEC/WAIT/STOP LED.
* Make a tool to create data for Virtual Disk.
* The GUI has too much strong dependencies on the Card themselves.
  Each change to a card causes recompilation, event when there's no change to the interface interesting for the GUI.
* For the IO Card, it would be better to address I/O by their logic number by type (I or O)
  instead of the absolute index.
* Make a Debuggers for GUI information (but can be use in CLI also)
    * Debugger that references memory, to display memory cards information
    * Debugger that references all cards to associate to specific debug panel.
* On Memory Display, show from which card the data comes from.
* Configurable RAM/ROM access time. Could also be different depending on RAM/ROM for mixed cards.
* Change the way remanence works, by scheduling regularly and "adding"/"subtracting" to a logic counter
  that is then read by the LED representation.
* Add configuration of the RTC
    * Frequency
    * Optional connection to BI7
* Shouldn't MemoryView be a range ?
* Create a test ROM the check the behavior of the 8008.
* A detection of cards that let their initialized schedule to default
  * The default it at zero, but a lot of cards starts unscheduled.
  * When they are unscheduled and have no step action, the Scheduler runs "void" and
    the application freezes without warning
  * INC Memory and DEC Memory are considered invalid by the disassembly. But how does it
    work on the 8008 execution?
* The bus request mechanism is interesting for studying, but it doesn't correspond to
  reality. The collisions could be detected differently, by every candidate asking for
  a value. All the values are "ORd" (AND on real hardware). If the resulting value
  is not the same, it means there's a collision. The collision can be reported
  to a service that decides if it's ok or not, and log or throw.

### Build

* Create a Standalone bundle exporter

### Possible optimization

* Implement profiler / Document profiling
* SignalRecorder makes a lot of costly memcpy.
    * This is the top 1 when profiling
* The Panels (Panels/Disassembly) are constantly re-updated, which is slow (especially for the PanelMemory)
    * This is high in the profiling
* Access to Debug Memory/Disassembly makes a lot of redundant computation
    * when getting the data
    * when computing the display
* Scheduler
    * Change the sort everything into a forward only sort. Or a dumb selection... Or something else
       * Maybe preselect until "unscheduled" then sort the subpart. (partition + sort)
    * Globally, the back signaling on every timing change is not efficient nor practical not pretty.
      * If Re-sort after each step with partition + sort, is the callback necessary?
* Signals are often created by shared_ptr and could benefit from a dedicated allocator to
  ensure locality (to profile first)
* 8008: probably the priority queue is overkill too.
* InstructionTableFor8008 for the disassembly and for the 8008 decoding are duplicated in memory.

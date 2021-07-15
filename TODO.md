### Implementation

* Panel and Console Card implementation
  * Implement the SUB mechanism
    * Place the SUB status from the ConsoleCard.
    * Add representation of the SUB signal from the Pluribus
    * Memory Cards should not place data on the bus when SUB is HIGH
    * ConsoleCard must place the set data on the bus when SUB is HIGH
  * Implement the AV switch (Need to understand better)
  * Implement both back panel switches
* Add a script system to manipulate the simulation
  * Can read from a console
  * Can read from a file
  * Commands can be:
    * (g)o (for simulator)
    * (p)ause (for simulator)
    * (s)et switch / data
    * (w)ait for status
* Implement the Serial card
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
* Add an Instruction Factory returning coded bytes, used by the Interrupt System opcode jamming (will also be useful for
  an assembler)
* Add an assembler project with an objective of compiling SCELBAL
* Implement a possibility to subscribe to a specific bit on an OwnedValue?
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
* The non-PLURIBUS with I/O cards should be ordered: card subscribing are asking.
  Signals should never be "pushed". Currently, some are pushed, some are subscribed.
* Use the LED remanence system on the EXEC/WAIT/STOP LED.
* Make a tool to create data for Virtual Disk.
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
* Probably the Memory Card mask could be collapsed to 2 or 3 bits in a byte, rather than
  three booleans. It would be simpler to write, and maybe produce smaller code (but
  probably not much improvement on this side.)
* A detection of cards that let their initialized schedule to default
  * The default it at zero, but a lot of cards starts unscheduled.
  * When they are unscheduled and have no step action, the Scheduler runs "void" and
    the application freezes without warning
  * INC Memory and DEC Memory are considered invalid by the disassembly. But how does it
    work on the 8008 execution?

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

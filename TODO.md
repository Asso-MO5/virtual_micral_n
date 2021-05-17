### Implementation

* Create an Auto Signal that changes states after some times, and have a schedule
* Implement the Stack card
  * Channel Mode (still needs to be understood)
* Implement the Unknown Device
  * Implement delays (on reading disk)
  * Add a way to set input data
  * Understand the CRC
  * Make a tool to create data
* Implement the I/O cards
  * Implement the Interrupts for the I/O cards
* Add an Instruction Factory returning coded bytes, used by the Interrupt System opcode jamming (will also be useful for
  an assembler)
* Panel and Console Card implementation
  * Implement the SUBST mechanism
  * Implement the AV switch (Need to understand better)
  * Implement both back panel switches
* Implement the Serial card
* Implement a possibility to subscribe to a specific bit on an OwnedValue?

### Architecture

* MemoryViewer system to verify. It's weird to have it in mcs8 library
* PluribusHelper, shouldn't it be MCS8 Helper ?

### Improvements

* Use the LED remanence system on the EXEC/WAIT/STOP LED.
* Make a Debuggers for GUI information (but can be use in CLI also)
    * Debugger that references memory, to display memory cards information
    * Debugger that references all cards to associate to specific debug panel.
* On Memory Display, show from which card the data comes from.
* Configurable RAM/ROM access time. Could also be different depending on RAM/ROM for mixed cards.
* Add configuration of the RTC
    * Frequency
    * Optional connection to BI7

### Build

* Create a Standalone bundle exporter

### Possible optimization

* SignalRecorder makes a lot of costly memcpy.
    * This is the top 1 when profiling
* The Panels (Panels/Disassembly) are constantly re-updated, which is slow (especially for the PanelMemory)
    * This is high in the profiling
* Access to Debug Memory/Disassembly makes a lot of redundant computation
    * when getting the data
    * when computing the display
* Scheduler
    * Change the sort everything into a forward only sort. Or a dumb selection... Or something else
    * Globally, the back signaling on every timing change is not efficient nor practical not pretty.
* 8008: probably the priority queue is overkill too.
* InstructionTableFor8008 for the disassembly and for the 8008 decoding are duplicated in memory.

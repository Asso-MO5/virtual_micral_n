* Implement the Interrupt System on the Processor Board
  * Clean the big switch cases
  * First instruction after a RST should not be interruptable.
  * Implement Masking
  * Step blocks when entering RST 38 : probably because of the first instruction gets interrupted.
* Why the values on D0-D7 when jamming an RST 0 by INIT?
* Add an Instruction Factory returning coded bytes, used by the Interrupt System opcode jamming (will also be useful for an assembler)
* Panel and Console Card implementation
    * Implement the SUBST mechanism
    * Implement the AV? switch (Need to understand better)
* Implement the Stack card
* Implement the I/O cards
* Once the interrupt system and I/O cards are there, implement the watch-dog.
* Use the LED remanence system on the EXEC/WAIT/STOP LED.
* Make a Debuggers for GUI information (but can be use in CLI also)
  * Debugger that references memory, to display memory cards information
  * Debugger that references all cards to associate to specific debug panel.
* On Memory Display, show from which card the data comes from.
* Add configuration of the RTC
  * Frequency
  * Optional connection to BI7
* Create a Standalone bundle exporter
* Extract 8008 tools to a lib, to ease writing an assembly/disassembly
* Configurable RAM/ROM access time. Could also be different depending on RAM/ROM for mixed cards.
* Extract 8008 instruction decoding so it can also serve in a disassembler or other tools
* Possible optimization
    * Scheduler: Change the sort everything into a forward only sort. Or a dumb selection... Or something else
        * Globally, the back signaling on every timing change is not efficient nor practical not pretty.
    * 8008: probably the priority queue is overkill too.
    * SignalRecorder makes a lot of costly memcpy.
    * InstructionTableFor8008 for the disassembly and for the 8008 decoding are duplicated in memory.
    * The Panels (Panels/Disassembly) are constantly re-updated, which is slow (especially for the PanelMemory)
    * As each board reacts to the synchro signal, these could be directly called functions rather than subscribed
      callbacks.
    * Access to Debug Memory/Disassembly makes a lot of redundant computation
        * when getting the data
        * when computing the display

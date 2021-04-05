* Panel and Console Card implementation
    * Implement the State machine of the different modes.
    * AUTO: Ok
    * STEP: Ok
    * AUTO/STEP on Startup mode: Ok
    * TRAP: Ok
    * SUBST
    * AV? (Need to understand better)
* The Ready C signal is "jumping" on display after its first change. Probably wrongly initialized.
* Implement the Interrupt System on the Processor Board
  * Improve the InterruptAtStart as the Interrupt circuit of the Processor Board
* Implement the Stack card
* Implement the I/O cards
* Make a Debuggers for GUI information (but can be use in CLI also)
  * Debugger that references memory, to display memory cards information
  * Debugger that references all cards to associate to specific debug panel.
* On Memory Display, show from which card the data comes from.
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

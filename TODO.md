* Populate the Pluribus with OwnedSignal
    * Communicate first with the Console Card
        * Implement the State machine of the different modes.
        * AUTO: Ok
        * STEP: Ok (but Check the data displayed on Panel)
        * On VDD up for the ConsoleCard, check the mode to select between AUTO and STEP
        * TRAP
        * SUBST
        * AV? (Need to understand better)
* Implement the Stack card
* Implement the I/O cards
* Make a Debuggers for GUI information (but can be use in CLI also)
  * Debugger that references memory, to display memory cards information
  * Debugger that references all cards to associate to specific debug panel.
* On Memory Display, show from which card the data comes from.
* Improve the InterruptAtStart as the Interrupt circuit of the Processor Board
* Create a Standalone bundle exporter
* Extract 8008 tools to a lib, to ease writing an assembly/disassembly
* Configurable RAM/ROM access time. Could also be different depending on RAM/ROM for mixed cards.
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

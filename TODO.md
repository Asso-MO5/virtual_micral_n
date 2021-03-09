* Optionally install the recorders on the Cards and Pluribus.
  * Will Need a way to unsubscribe to signals at some point.
* Add a kind of signal to show states (data bus, Cpu States...)
* Populate the Pluribus with OwnedSignal
    * Communicate first with the Console Card
        * Implement the State machine of the different modes.
        * AUTO: Ok
        * STEP: Is broken. Try to find the good signal/timings.
        * On VDD up for the ConsoleCard, check the mode to select between AUTO and STEP
        * TRAP
        * SUBST (Needs new Memory Cards)
        * AV? (Need to understand better)
    * Possibility to set Debug Read Information on the Processor Board
    * Except on the Processor Bus, other components should not address it directly, but through the Pluribus (except for
      Debug Feedback)
* Implement the I/O cards
* Move the DataBus to the ProcessorCard to share with the CPU
    * Change to OwnedSignal?
* Control Panel + Simulator fixes to work with the Console Panel
    * The debug at Simulator level doesn't work when Stepping on the Emulated system
* Make a Debuggers for GUI information (but can be use in CLI also)
  * Debugger that references memory, to display memory cards information
  * Debugger that references all cards to associate to specific debug panel.
* On Memory Display, show from which card the data comes from.
* Show more signals on the Debug Panel (those on the pluribus).
  * Dedicate a Panel to collect and show Signals
  * Add a view for OwnedValues as signals.
* Improve the InterruptAtStart as the Interrupt circuit of the Processor Board
* Are OwnedValue and ConnectedData redundant or do they serve different things?
* Extract the definition of the machine from the Simulator
* Clean: extract Recorders from the Simulator
* Update the CLI to a valid configuration
    * Remove SimpleROM/SimpleRAM and ControlBus from the project.
* Make the clock widget only appear on pause?
* Create a Standalone bundle exporter
* Extract 8008 tools to a lib, to ease writing an assembly/disassembly
* There are missing bound checks, sometimes, the simulator stops because vdd is LOW. But it's not set low. So there's
  probably an out of bound access.
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

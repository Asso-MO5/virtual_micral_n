* Change the get_state() from OwnedValue to get_value()
* Populate the Pluribus with OwnedSignal
  * Communicate first with the Console Card
    * Implement the State machine of the different modes.
    * AUTO
    * STEP
    * TRAP
    * SUBST
    * AV?
  * Will need to deactivate the temp IO Controller, RAM and Control Bus.
    ... or not.
  * Possibility to set Debug Read Information on the Processor Board
  * Except on the Processor Bus, other components should not address it
  directly, but through the Pluribus (except for Debug Feedback)
* Plug the Control Panel to a Control Board
* Plug the control board to the pluribus
  * Pluribus must accept virtual "cards"
* Plug the CPU board to the pluribus
* Plug memories to the pluribus
* Improve the InterruptAtStart as the Interrupt circuit of the Processor Board
* Are OwnedValue and ConnectedData redundant or do they serve different things?    
* Extract the definition of the machine from the Simulator
* Clean: extract Recorders from the Simulator
* Make the clock widget only appear on pause?
* Create a Standalone bundle exporter
* Extract 8008 tools to a lib, to ease writing an assembly/disassembly
* There are missing bound checks, sometimes, the simulator stops because vdd is LOW.
  But it's not set low. So there's probably an out of bound access. 
* Possible optimization  
  * Scheduler: Change the sort everything into a forward only sort. Or a dumb selection... Or something else
  * 8008: probably the priority queue is overkill too.
  * SignalRecorder makes a lot of costly memcpy.
  * InstructionTableFor8008 for the disassembly and for the 8008 decoding are duplicated in memory.
  * The Panels (Panels/Disassembly) are constantly re-updated, which is slow (especially for the PanelMemory)
  * As each board reacts to the synchro signal, these could be directly called functions
  rather than subscribed callbacks.

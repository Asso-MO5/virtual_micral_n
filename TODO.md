* PCW and PCC
    * On a PCW cycle, T3 must be a DATA OUT
    * There is also a Write Cycle on T4 for the Conditions with PCC
* Wait for READY on T3 of Cycle 2 of OUT
* Implement Simple RAM
* Implement a disassembly window
* Implement a RAM inspector window
* Add the copy string example as loadable ROM + RAM
* Extract the definition of the machine from the Simulator
* Clean: extract Recorders from the Simulator
* Make the clock widget only appear on pause?
* Create a Standalone bundle exporter
* Extract 8008 tools to a lib, to ease writing an assembly/disassembly
* Possible optimization  
  * Scheduler: Change the sort everything into a forward only sort. Or a dumb selection... Or something else
  * 8008: probably the priority queue is overkill too.
  * SignalRecorder makes a lot of costly memcpy.
  * InstructionTableFor8008 for the disassembly and for the 8008 decoding are ducplicated in memory.
  
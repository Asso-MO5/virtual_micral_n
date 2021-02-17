* Implement Simple RAM
* Implement a RAM inspector window
* Implement OUT
  * Implement PCC Cycle
  * Wait for READY on T3 of Cycle 2 of OUT
* Add the copy string example as loadable ROM + RAM
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
  * InstructionTableFor8008 for the disassembly and for the 8008 decoding are ducplicated in memory.
  
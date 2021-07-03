### Implementation

* Create a debug panel for the DiskController
  * Show the status
  * Show a log
* Simplify writing the connections in the connectors
* Puts a configuration with both ROMs in place.
* Implement the ROM/RAM Switching on the 4k RAM/ROM card. 
* Implement the Stack card
    * Channel Mode (still needs to be understood)
* Implement the Unknown Device (which is the Disk Controller)
    * Implement delays (on reading disk)
    * Add a way to set input data
    * Implement tool for computing the CRC
      ```c++
        const auto data = disk_data[status.index_on_disk];
        const auto crc = crc_of_sent_data;
        const auto for_carry =
                ((data ^ crc) ^ 0xff) & ((data + crc) ^ data);
        const std::uint8_t carry = (for_carry & 0x80) ? 1 : 0;
        crc_of_sent_data = crc + data + carry;
      ```
    * Make a tool to create data
* Change the DiskController implementation with the latest analysis.
  * The controller doesn't seem to need an I/O card.
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
* Change LED colors to correctly matching the hardware.

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
* Add a clang-tidy configuration
* Sets cppcheck and clang-tidy to a CI
* The non-PLURIBUS with I/O cards should be ordered: card subscribing are asking.
  Signals should never be "pushed". Currently, some are pushed, some are subscribed.
* Use the LED remanence system on the EXEC/WAIT/STOP LED.
* Make a Debuggers for GUI information (but can be use in CLI also)
    * Debugger that references memory, to display memory cards information
    * Debugger that references all cards to associate to specific debug panel.
* On Memory Display, show from which card the data comes from.
* Configurable RAM/ROM access time. Could also be different depending on RAM/ROM for mixed cards.
* Add configuration of the RTC
    * Frequency
    * Optional connection to BI7
* Shouldn't MemoryView be a range ?

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
       * Maybe preselect until "unscheduled" then sort the subpart. (partition + sort)
    * Globally, the back signaling on every timing change is not efficient nor practical not pretty.
      * If Re-sort after each step with partition + sort, is the callback necessary?
* Signals are often created by shared_ptr and could benefit from a dedicated allocator to
  ensure locality (to profile first)
* 8008: probably the priority queue is overkill too.
* InstructionTableFor8008 for the disassembly and for the 8008 decoding are duplicated in memory.

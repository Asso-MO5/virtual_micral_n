### Emulation

* Serial Card
* TTY
    * Gets punched data emitted to the TTY
        * Automatically capture on DC2 if "record" mode
    * Sends punched data from the TTY
    * The TTY Panel has options to
        * Play the data
        * Record the data
        * Save and Load the ribbon content
        * See the data on the ribbon
        * Modify the data on the ribbon
    * When launching the CLI, the TTY is attached to the terminal
        * Make a specific terminal version for better key acquisition, defaulting on the standard C++ version.
    * A backspace in the input buffer crashes the Basic. Check why.
    * The Power Function '^' is not recognized (probably not mapped to the correct key code)
* Console
    * Implement both back panel switches
        * Start mode
            * Needs to implement the startup system on the processor card correctly
        * Disabling
    * Change LED colors to correctly matching the hardware.
    * LED remanence
        * Change the way remanence works, by scheduling regularly and "adding"/"subtracting" to a logic counter that is
          then read by the LED representation.
        * Use the LED remanence system on the EXEC/WAIT/STOP LED.
* Processor Card
    * Add configuration of the RTC
        * Frequency
        * Optional connection to BI7 (at Simulator construction, this cannot be changed on the fly)
* Memory Card
    * Improve the display of Memory
        * Telling the panel data has changed, rather that constant redisplay.
        * Same for Disassembler view
    * Implement the Console Switch modification
    * On Memory Display, show from which card the data comes from.
    * Configurable RAM/ROM access time. Could also be different depending on RAM/ROM for mixed cards.
* Stack Card
    * Implement the Channel Mode when it's understood.
    * Show write status on the GUI when implemented.
* Disk Controller
    * Add controllable delays on reading the disk
        * DiskReader can be a Schedulable to emulate the reading/writing times
* I/O cards
    * Implement the Interrupts for the I/O cards
* Pluribus viewer
    * Gives the option to Inverse the signal
* Power
    * Emulate the VDD presence.
        * Needs to implement the startup system on the processor card correctly
    * Add a Power On/Off on the GUI
* Make the construction of the Simulator data drivable (by commands? configuration file?)

### Guided Presentation project

* Implement a project that displays a reconstructed Micral N
* Create a Presentation System with directive and dialogs for the user.
    * The goal is to create scenarios presenting the Micral N and allowing guided manipulation.
* The Switch should move
* The LED should blink
* The TTY should make sound.
* The Disk Drive should make sound.

### Tools

* Add a script system to manipulate the simulation
    * Can read from a console
    * Can read from a file
    * Commands can be:
        * (g)o (for simulator)
        * (p)ause (for simulator)
        * (s)et switch / data
        * (w)ait for status
        * loads a setup for the emulator
* Disassembler
    * Add option to see octal in the disassembly
* Make a tool to create data for Virtual Disk (for real hardware, so wait for it)
* Make a Debuggers for GUI information (but can be use in CLI also)
    * Debugger that references memory, to display memory cards information
    * Debugger that references all cards to associate to specific debug panel.

### Implementation

* Implement a possibility to connect OwnedSignals to a concentrated OwnedValue (for IOCard/SerialCard)
    * At the moment, the Serial Card "cheats" by hardcoding the signals in a status word
* Add an Instruction Factory returning coded bytes, used by the Interrupt System opcode jamming (will also be useful for
  an assembler)
* Add an assembler project with an objective of compiling SCELBAL
    * Then add Macro to allow programming easier
* Implement a possibility to subscribe to a specific bit on an OwnedValue? (where was it needed?)
* MemoryViewer system to verify. It's weird to have it in i8008 library
* PluribusHelper, shouldn't it be i8008 Helper ?
* Fix the TODO's
* Static Analysis
    * Remove the cppcheck warnings
    * Add a clang-tidy configuration
    * Sets cppcheck and clang-tidy to the CI
* Most of the signals react on a specific edge. Add possibility to subscribe to a specific edge.
    * Check performance before/after
* The GUI has too much strong dependencies on the Card themselves. Each change to a card causes recompilation, event
  when there's no change to the interface interesting for the GUI.
* For the IO Card, it would be better to address I/O by their logic number by type (I or O)
  instead of the absolute index.
* INC Memory and DEC Memory are considered invalid by the disassembly. But how does it work on the 8008 execution?
* A detection of cards that let their initialized schedule to default
    * The default it at zero, but a lot of cards starts unscheduled.
    * When they are unscheduled and have no step action, the Scheduler runs "void" and the application freezes without
      warning
* The bus request mechanism is interesting for studying, but it doesn't correspond to reality. The collisions could be
  detected differently, by every candidate asking for a value. All the values are "ORd" (AND on real hardware). If the
  resulting value is not the same, it means there's a collision. The collision can be reported to a service that decides
  if it's ok or not, and log or throw.
* Keep only one #ifdef EMSCRIPTEN with better architecture (GUI main.cpp and ImGuiSDLGLContext.h)

### Documentation

* Add Architecture.md in documentation/ explaining and linking to local library documentation and with a dependency
  diagram (generated from cmake?).
* Add a check that parse every README.md from libraries and check that names in backquotes are actual names found in the
  code. Also check that every class name in the code is referenced at least once in a (local?) README.md. It could also
  produce a cross-reference file.

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
    * Globally, the back signaling on every timing change is not efficient nor practical not pretty.
        * If Re-sort after each step with partition + sort, is the callback necessary?
* Signals are often created by shared_ptr and could benefit from a dedicated allocator to ensure locality (to profile
  first)
* 8008 probably the priority queue is overkill too. Use schedules actions?
* InstructionTableFor8008 for the disassembly and for the 8008 decoding are duplicated in memory.

### Program Ideas

* Create a test ROM to check the behavior of the 8008.
* A P-Code virtual machine
* A 8008 Forth (the Stack Card can help)

### Build

* Uses CMake Presets
    * Adapt for a MacOS build
    * Adapt for a Windows build
* GitHub workflows
    * Add Windows and Emscripten builds
    * Add Packaging

### Questions

* Should pressing INIT switch to RAM bank while loading?

# XM-23 Machine Emulator

# Overview
This project is a C-based console emulator for the XM-23 Reduced Instruction Set Computer (RISC), a machine designed by Dr. Larry Hughes at Dalhousie University. The emulator replicates the key architectural features of the XM-23, providing a hands-on learning experience for students studying computer architecture.

# Features
- Fetch-Decode-Execute Cycle: Implements the core instruction cycle, enabling the sequential execution of instructions as specified in the XM-23 architecture.
- Memory Management: Supports 64 KB of primary memory and 32 bytes of cache memory with write-back and write-through policies.
- Interrupt Handling: Handles interrupts, faults, and traps with their respective handlers stored in the interrupt vector table.
- Device Support: Emulates up to 8 memory-mapped devices, including a programmable timer, keyboard, and screen.
- Step-Through Execution: Allows the user to execute programs one instruction at a time, with detailed monitoring of the CPU registers, memory, and cache.
- Breakpoint Setting: Users can set breakpoints at specific memory addresses to halt execution and inspect the system state.

# Usage
- Compile the Code: Ensure that your development environment supports C and compile the source files.
- Run the Emulator: Load a program using Motorola S-Record (SREC) files and execute it within the emulator.
- Monitor Execution: Use the step-through execution mode, set breakpoints, and monitor the CPU and memory state to gain insights into low-level operations.

# Files
- CPU.c / CPU.h: Handles the core CPU operations, including instruction fetching, decoding, and execution.
- Memory.c / Memory.h: Manages memory operations, including cache implementation.
- Exceptions.c / Exceptions.h: Handles different types of exceptions like interrupts and faults.
- UserInterface.c / UserInterface.h: Provides console-based interaction with the emulator.

# Getting Started
Clone the repository, navigate to the project directory, and compile the source files. Use the provided tools to load and execute programs, and explore the low-level workings of the XM-23 machine.

# Acknowledgments
Special thanks to Dr. Larry Hughes for his guidance and the inspiration behind this project.

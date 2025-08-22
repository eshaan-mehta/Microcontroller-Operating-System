# STM32 Microcontroller Real-Time Operating System

## Overview

This project implements a preemptive real-time operating system (RTOS) for an STM32 Nucleo development board. The OS features a custom kernel with task scheduling, dynamic memory management, and deadline-based preemption, all built using **ARM Cortex-M4**, **Assembly** and **C**.

## System Architecture

### 1. Kernel

To manage all the tasks and allow for multitasking, all task metadata _(TCB)_ is stored in a buffer.

Tasks can exist in several states:
- **DORMANT**: The task has been created but is not yet ready to run.
- **READY**: The task is ready to run and is waiting for the scheduler to give it CPU time.
- **RUNNING**: The task is currently executing on the CPU.
- **SLEEPING**: The task has been temporarily suspended, usually to wait for an event or for a specific duration.

Kernel methods is interfaced using its API, allowing for task creation, termination, and synchronization. These methods are accessed via system calls using _SVC_

### 2. Preemptive Scheduling and Context Switching

The OS implements **preemptive multitasking**, allowing for  higher-priority tasks to take precedance as they come up. This is done using the **Earliest Deadline First (EDF)** algorithm in the task scheduler.

Context switching is done using the the _PendSV interrupt_. This saves the current task's state to its stack and loads the new task's state, allowing the new task to resume exactly where it left off.

### 3. Dynamic Memory Management

The memory manager uses a **first-fit allocation algorithm**. When a task requests memory, the manager scans a list of free blocks and allocates the first one that is large enough. To combat fragmentation, adjacent free blocks are automatically coalesced into a single, larger block.

The memory map is carefully defined, with distinct regions for program code (Flash), data (SRAM), and memory-mapped peripherals.

To ensure thread safety, every allocated memory block is tagged with the ID of the task that requested it. This prevents one task from accidentally freeing another task's memory and allows the kernel to automatically clean up all memory allocated by a task when it exits, preventing memory leaks.

### 4. System Call Interface

To maintain system stability and security, tasks operate in an **unprivileged mode**, with limited access, while the kernel runs in **privileged mode** with full system access. During a system call the SVC handler executes the requested method in privileged mode, and safely returns the result. 


## Key Design Decisions

The architecture of this RTOS was guided by several key design principles tailored for real-time embedded systems:

-   **Preemption over Cooperation**: A preemptive scheduler was chosen to provide strong real-time guarantees. Unlike cooperative multitasking, where a task must voluntarily give up control, preemption ensures that high-priority tasks can run exactly when needed.
-   **Layered Interrupt Architecture**: A multi-level interrupt priority scheme is used to create a robust and responsive system. By giving system calls the highest priority, the kernel can execute critical operations without being interrupted.

## Building and Testing the System

The project is configured for development with _STM32CubeIDE_ and the _ARM GCC_ toolchain.

-   **Linker Script**: A custom linker script (`STM32F401RETX_FLASH.ld`) lays out how the program is organized in memory. It defines the locations for the program code, the kernel's stack, the dynamic memory heap.
-   **Startup Code**: The assembly startup file (`startup_stm32f401retx.s`) is the first code to run on boot. It sets up the initial stack pointer, initializes the C runtime environment, and then jumps to the `main()` function to start the OS.


## Testing

-   **Memory Testing**: Stress tests verify the robustness of the memory allocator, checking for memory leaks, fragmentation issues, and correct behavior under heavy load.
-   **Timing Analysis**: The performance of critical operations like memory allocation and context switching is measured to ensure the system meets its timing requirements.
-   **Task Management Tests**: These tests confirm that tasks are created, scheduled, and terminated correctly, and that the scheduler's logic is sound.
-   **Preemption Validation**: Real-world scenarios are simulated to verify that deadline-driven tasks correctly preempt lower-priority tasks, proving the effectiveness of the real-time scheduler.

This project serves as a practical demonstration of advanced embedded systems concepts, including real-time scheduling, dynamic memory management, and hardware-software co-design on a resource-constrained microcontroller platform.

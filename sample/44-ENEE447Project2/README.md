
# Project 2: Preemptive Multitasking (initiated by timer interrupt)

  

## What are we doing in this project
- We will modify circle so that it supports preemptive multitasking initiated by timer interrupt.
	- What is preemptive multitasking? According to [Wikipedia](https://en.wikipedia.org/wiki/Preemption_(computing)#Preemptive_multitasking): "In simple terms: Preemptive multitasking involves **the use of an interrupt mechanism which suspends the currently executing process and invokes a scheduler** to determine which process should execute next. Therefore, all processes will get some amount of CPU time at any given time."
- **NOTE**: For this project, our code will target qemu-emulated Raspberry Pi Zero (which means the code may not run on your real physical Pi). 
	- Read FAQ1 in project 1 to learn how to run qemu-emulated Raspberry Pi Zero on your computer.

## Specifically, we are trying to solve the following problem:
- Currently, when we run this sample, the output shows only main task is running:
```
logger: Circle 45.1 started on Raspberry Pi Zero
00:00:00.54 timer: SpeedFactor is 1.85
00:00:00.54 kernel: Compile time: Feb 25 2023 15:03:24
Task main is running.
Task main is running.
Task main is running.
Task main is running.
Task main is running.
Task main is running.
Task main is running.
```

- After we implement preemptive multitasking (initiated by timer interrupt), the output should show four tasks concurrently running (main task, task A, task B, and task C):
```
logger: Circle 45.1 started on Raspberry Fi Zero
00:00:00.54 timer: SpeedFactor is 1.85
00:00:00.54 kernel: Compile time: Feb 25 2023 15:03:24
Task main is running.
00:00:01.54 sched: Current task is task main, will switch to task A.
Task A is running.
DO:00:02.54 sched: Current task is task A, will switch to task B.
Task B is running.
00:00:03.54 sched: Current task is task B, will switch to task C.
Task C is running.
00:00:04.54 sched: Current task is task C, will switch to task main.
Task main is running.
Task main is running.
00:00:05 54 sched: Current task is task main, will switch to task A.
Task A is running.
Task A is running.
```

## Specifically, you are supposed to do the following:
1. Read the [`Run` function in `kernel.cpp`](kernel.cpp#L78-L116) and the [`Run` function in `testtask.cpp`](testtask.cpp#L17-L34).
	- Note that, compared to project 1, none of these tasks in project 2 would call `Yield` (and they run forever in a `while(1)` loop). This means multitasking among them can't be achieved through cooperative multitasking because they never yield and never exit. Therefore, to achieve multitasking in project 2, we need preemptive multitasking. 
1. The starting point of preemptive multitasking is [the call to `EnablePreemptiveMultitasking` in `kernel.cpp`](kernel.cpp#L98-L99). Read [the definition of `EnablePreemptiveMultitasking` in `scheduler.cpp`](../../lib/sched/scheduler.cpp#L466-L480).
	- Note that, besides initializing some global variables, `EnablePreemptiveMultitasking` calls `RegisterPeriodicHandler` to register `a_simple_timer_interrupt_handler`. This means from this point forward, `a_simple_timer_interrupt_handler` will be called on every timer interrupt.
1. Read [`a_simple_timer_interrupt_handler` in `scheduler.cpp`](../../lib/sched/scheduler.cpp#L452-L463).
	- **NOTE**: Context switch does not happen in this function because some other interrupt-handling code may need to run after this function (for example, another registered timer interrupt handler). Therefore, if the function decides a context switch should happen, it simply sets the flag `should_contextswith_on_irq_return`, which will affect [how `IRQStub` in `exceptionstub.S` exits from IRQ mode](../../lib/exceptionstub.S#L104-L112).
1. Read the code and comments of [`IRQStub` in `exceptionstub.S`](../../lib/exceptionstub.S#L80) and **implement the `TODO`s under the label `do_context_switch_on_irq_return`**.  
1. Read the code and comments of [`ContextSwitchOnIrqReturn_by_modifyingTaskContextSavedByIrqStub` in `scheduler.cpp`](../../lib/sched/scheduler.cpp#L482) and **implement the `TODO` in the function**.  

## What to submit on ELMS before your lab in the week of Mar 13:
1. A pdf that has:
	- Members of your group.
	- A screenshot/photo that shows the four tasks concurrently running (main task, task A, task B, and task C).
1. The file `exceptionstub.S` in which you have implemented the `TODO`s under the label `do_context_switch_on_irq_return`.
1. The file `scheduler.cpp` in which you have implemented the `TODO` in the function `ContextSwitchOnIrqReturn_by_modifyingTaskContextSavedByIrqStub`.

## Important documents for reference
- [1] [ARM Architecture Reference Manual](https://documentation-service.arm.com/static/5f8dacc8f86e16515cdb865a)
- [2] [ARM1176JZF-S Technical Reference Manual](https://developer.arm.com/documentation/ddi0301/latest/)
	- ARM1176JZF-S is the processor used in Raspberry Pi Zero according to [here](https://www.raspberrypi.com/documentation/computers/processors.html).
	-  **NOTE: This ARM processor supports two ISAs -- the original [ARM ISA](https://en.wikipedia.org/wiki/ARM_architecture_family#Instruction_set), which we will be using for this project, and [Thumb ISA](https://en.wikipedia.org/wiki/ARM_architecture_family#Thumb). When we read these documents, we only need to read texts that are relevant to ARM ISA (texts that mention "ARM state") not thumb ISA (texts that mention "thumb state").**
		- For example, when trying to learn what registers are available to us, we should read the section ["The ARM state core register set"](https://developer.arm.com/documentation/ddi0301/h/programmer-s-model/registers/the-arm-state-core-register-set?lang=en) instead of the section ["The Thumb state core register set"](https://developer.arm.com/documentation/ddi0301/h/programmer-s-model/registers/the-thumb-state-core-register-set?lang=en)
- [3] [ARM Procedure Call Standard](https://developer.arm.com/documentation/dui0041/c/ARM-Procedure-Call-Standard)


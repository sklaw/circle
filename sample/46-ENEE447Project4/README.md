# Project 4 and 5: Running a task in user mode with its own virtual memory space

## What are we doing in this project?
In this project, we will try to run a task in user mode with its own virtual memory space.
Specifically, this means:
- The task won't be able to execute privilleged instructions.
- Multiple tasks can use the same memory region (for storing a task's binary codes, placing the task's stack, etc.) as long as their page tables map the memory region into different physical pages.

### Why do we want to do this?
- [Principle of least privilege](https://en.wikipedia.org/wiki/Principle_of_least_privilege): User codes can be malicious or buggy. By limiting its privileges, we can limit the damage it can cause.
- Better memory security: 
  - Tasks can no longer access each other's memory because their page tables map their memory space into differeny physical pages.
    - However, in some cases we do want tasks to share memory. For example:
      - If multiple tasks import the same library, we can save memory space by having one copy of the library in physical memory and make each task's page table map into it. This is called a [shared library](https://en.wikipedia.org/wiki/Library_(computing)#Shared_libraries).
      - Two tasks may want shared memory for [inter-process communication](https://en.wikipedia.org/wiki/Inter-process_communication).
      - ...
    - In this case, the OS kernel needs to implement a mechanism for shared memory (which we currently don't have).
  - We can mark pages that contain kernel code as privileged so that user code cannot access it.
  - ...
- ...

## Specifically, we are trying to solve the following two problems:

### Before you start, copy your project 3 solution to here and here.

### Problem 1: Set up virtual memory for the user mode task.
- Currently, when we run this sample, it will have an error:

<img src="img/project 4 after copying p3 sol_part 1 init state_Run called.png" width="500">

- After we have set up VM for the user mode task correctly, the sample should run without errors but also without output:

<img src="img/project 4 after impl vm_user task runs but no output_need syscall impl.png" width="500">

#### Specifically, to solve problem 1, you are supposed to do the following:
- Implement the following TODOs:
  -
  
### Problem 2: ...(TBD)

## What to submit on ELMS before your lab in the week of May 1 (you have 3 weeks):
1. A pdf that has:
	- Members of your group.
	- A screenshot/photo that shows problem 1 is solved.
	- A screenshot/photo that shows problem 2 is solved.
1. ... (TBD)
1. ... (TBD)

## Documents for reference
- [1] [ARM Architecture Reference Manual](https://documentation-service.arm.com/static/5f8dacc8f86e16515cdb865a)
- [2] [ARM1176JZF-S Technical Reference Manual](https://developer.arm.com/documentation/ddi0301/latest/)
	- ARM1176JZF-S is the processor used in Raspberry Pi Zero according to [here](https://www.raspberrypi.com/documentation/computers/processors.html).
	-  **NOTE: This ARM processor supports two ISAs -- the original [ARM ISA](https://en.wikipedia.org/wiki/ARM_architecture_family#Instruction_set), which we will be using for this project, and [Thumb ISA](https://en.wikipedia.org/wiki/ARM_architecture_family#Thumb). When we read these documents, we only need to read texts that are relevant to ARM ISA (texts that mention "ARM state") not thumb ISA (texts that mention "thumb state").**
		- For example, when trying to learn what registers are available to us, we should read the section ["The ARM state core register set"](https://developer.arm.com/documentation/ddi0301/h/programmer-s-model/registers/the-arm-state-core-register-set?lang=en) instead of the section ["The Thumb state core register set"](https://developer.arm.com/documentation/ddi0301/h/programmer-s-model/registers/the-thumb-state-core-register-set?lang=en)
- [3] [ARM Procedure Call Standard](https://developer.arm.com/documentation/dui0041/c/ARM-Procedure-Call-Standard)



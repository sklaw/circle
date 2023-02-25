/*
 * exceptionstub.S
 *
 * Circle - A C++ bare metal environment for Raspberry Pi
 * Copyright (C) 2014-2020 R. Stange <rsta2@o2online.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <circle/sysconfig.h>
#include <circle/exception.h>
#include <circle/bcm2835.h>

	.macro	stub name, exception, pc_offset

	.globl	\name
\name:
	sub	sp, sp, #4			/* correct stack (number of pushs must be even) */
	sub	lr, lr, #\pc_offset		/* lr: correct PC of aborted program */
	stmfd	sp!, {lr}			/* store PC onto stack */
	mrs	lr, spsr			/* lr can be overwritten now */
	stmfd	sp!, {lr}			/* store saved PSR onto stack */
	stmfd	sp, {r0-r14}^			/* store user registers r0-r14 (unbanked) */
	sub	sp, sp, #4*15			/* correct stack (not done by previous instruction */
	mov	r1, sp				/* save sp_abt or sp_und */
	cps	#0x12				/* set IRQ mode to access sp_irq and lr_irq */
	mov	r2, sp
	mov	r3, lr
	cps	#0x11				/* set FIQ mode to access sp_fiq and lr_fiq */
	mov	r4, sp
	mov	r5, lr
	cps	#0x1F				/* our abort handler runs in system mode */
	mov	sp, r1				/* set sp_sys to stack top of abort stack */
	stmfd	sp!, {r2-r5}			/* store lr_fiq, sp_fiq, lr_irq, sp_irq onto stack */
	mov	r1, sp				/* r1: pointer to register frame */
	mov	r0, #\exception			/* r0: exception identifier */
	b	ExceptionHandler		/* jump to ExceptionHandler (never returns) */

	.endm

	.text

/*
 * Abort stubs
 */
	stub	UndefinedInstructionInternal,	EXCEPTION_UNDEFINED_INSTRUCTION,	4
	stub	PrefetchAbortStub,		EXCEPTION_PREFETCH_ABORT,		4
	stub	DataAbortStub,			EXCEPTION_DATA_ABORT,			8

/*
 * Undefined Instruction stub
 
 
 */
	.globl UndefinedInstructionStub
UndefinedInstructionStub:
	stmfd	sp!, {r0, lr}			/* save r0 and return address */
	fmrx	r0, fpexc			/* check for floating point exception */
#define VFP_FPEXC_EX	(1 << 31)
	tst	r0, #VFP_FPEXC_EX		/* if EX bit is clear in FPEXC */
	beq	UndefinedInstructionInternal	/* then jump to abort stub */
	bic	r0, r0, #VFP_FPEXC_EX		/* else clear EX bit */
	fmxr	fpexc, r0
	ldmfd	sp!, {r0, pc}^			/* restore registers and return */

/*
 * IRQ stub
 */
	.globl	IRQStub
IRQStub:
	// To understand IRQStub, you need to first understand the following topics: 
	//   1. What is "IRQ mode"? Read "A2.2 Processor modes", "A2.3 Registers", 
	//     "A2.4 General-purpose registers", and "A2.5 Program status registers" in [1].
	//   2. What happens when the CPU enters IRQ mode? Read "A2.6.8 Interrupt 
	//     request (IRQ) exception" in [1].
	//   3. Where can I look up asssembly instructions? Use "A4.1 Alphabetical list of 
	//     ARM instructions" in [1].

	sub	lr, lr, #4			/* lr: return address */
	stmfd	sp!, {r0-r3, r12, lr}		/* save r0-r3, r12 and return address */
#ifdef SAVE_VFP_REGS_ON_IRQ
	sub	sp, sp, #4			/* correct stack (number of pushs must be even) */
	vmrs	r0, fpscr			/* save VFP registers */
	stmfd	sp!, {r0}
	vstmdb	sp!, {d0-d15}
#if RASPPI >= 2 && defined (__FAST_MATH__)
	vstmdb	sp!, {d16-d31}
#endif
#endif
	ldr	r0, =IRQReturnAddress		/* store return address for profiling */
	str	lr, [r0]
	bl	InterruptHandler

	// Added by TA.
	// If should_contextswith_on_irq_return is set (which means 
	//   this IRQ was triggered by a timer interrupt and the interrupt handler decided the interrupted 
	//   task has used up its CPU time slice), we branch to `do_context_switch_on_irq_return`; 
	//   otherwise, we simply restore registers saved at the beginning of IRQStub and return.
	ldr	r0, =should_contextswith_on_irq_return
	ldr	r0, [r0]
	cmp	r0, #0
	bne	do_context_switch_on_irq_return

#ifdef SAVE_VFP_REGS_ON_IRQ
#if RASPPI >= 2 && defined (__FAST_MATH__)
	vldmia	sp!, {d16-d31}
#endif
	vldmia	sp!, {d0-d15}			/* restore VFP registers */
	ldmfd	sp!, {r0}
	vmsr	fpscr, r0
	add	sp, sp, #4			/* correct stack */
#endif
	ldmfd	sp!, {r0-r3, r12, pc}^		/* restore registers and return */
	
do_context_switch_on_irq_return:
#ifdef SAVE_VFP_REGS_ON_IRQ
#if RASPPI >= 2 && defined (__FAST_MATH__)
	vldmia	sp!, {d16-d31}
#endif
	vldmia	sp!, {d0-d15}			/* restore VFP registers */
	ldmfd	sp!, {r0}
	vmsr	fpscr, r0
	add	sp, sp, #4			/* correct stack */
#endif
	ldmfd	sp!, {r0-r3, r12, lr}		/* restore registers but __DO NOT__ return */
	// The above 6 instructions are identical to the 6 instructions 
	//   before them except that the last instruction replaced "pc" with "lr"
	//   and removed the ending caret "^". 
	//
	// This change is significant because `ldmfd sp!, {r0-r3, r12, pc}^`
	//   triggers a return from IRQ mode to whatever mode that was running 
	//   before the interrupt occurred, while `ldmfd sp!, {r0-r3, r12, lr}` does not.
	//
	// Specifically, `ldmfd sp!, {r0-r3, r12, pc}^` will copy spsr_irq 
	//  into cpsr**, which effectively restores the processor state to the time before the 
	//  interrupt occurred. If before the interrupt occurred, the interrupted task was running with IRQ
	//  interrupt enabled, this act effectively re-enables IRQ interrupt! 
	// 	
	// * Read "A4.1.22 LDM (3)" and "A4.1.21 LDM (2)" in [1] for details.
	// ** Read "A2.5 Program status registers" in [1] for details.


	// TA's note: At this point, all registers (except pc) have been restored to 
	//   the values they had when IRQStub just entered, meaning we now have access to the
	//   full context of the interrupted task.

	// Here is what we are going to do next:
	//   1. We will save the context (registers) of the interrupted task on the stack.
	//     - specifically, we need to save r0-r15, d0-d15, fpexc, and fpscr 
	//       of the interrupted task. We don't need to save its cpsr for this project
	//       because all tasks have the same cpsr (they all run in system mode with IRQ
	//       interrupt enabled).
	//   2. When we save the registers on the stack, we will organize them in a way such that
	//      we can manipulate them later in C code as a `struct TTaskRegisters` (read the struct 
	//      declaration in include/circle/sched/taskswitch.h).
	//     - Specifically, this means, we need to store the registers declared in
	//       `struct TTaskRegisters` consecutively on the stack, with the first field (fpexc)
	//       stored at the lowest memory address and the last filed (pc) stored at
	//       the highest memory address.

	// Allocate space on stack for saving interrupted task's r15/pc.
	sub	sp, sp, #4
	// Save interrupted task's r15/pc in alloated space
	stmia	sp, {lr} // Why are we saving lr instead of pc? 
	                 // Read "A2.6.8 Interrupt request (IRQ) exception" in [1]. 

	// Allocate space on stack for saving interrupted task's r0-r14
	sub	sp, sp, #60
	// TODO: Save interrupted task's r0-r14 in alloated space

	// Allocate space on stack for saving interrupted task's d0-d15
	sub	sp, sp, #128
	// TODO: Save interrupted task's d0-d15 in alloated space

	// Allocate space on stack for saving interrupted task's fpexc and fpscr.
	sub	sp, sp, #8
	// TODO: Save interrupted task's fpexc and fpscr in alloated space


	
	// Next, we call the function `ContextSwitchOnIrqReturn_by_modifyingTaskContextSavedByIrqStub`
	//   with one argument (passed in r0). The function requires that the argument passed in
	//   should be a pointer of `struct TTaskRegisters` that points to the saved context. 
	//   Hence, we will first copy sp to r0 because sp at this moment is pointing to the saved context.
	// mov	r0, sp
	// bl	ContextSwitchOnIrqReturn_by_modifyingTaskContextSavedByIrqStub
	// NOTE: If you haven't implemented and tested the TODOs in this file, 
	//   the above two lines could irreversibly overwrite registers and thus destroy the context of the 
	//   interrupted task. Therefore, you should uncomment the above two lines only after you have 
	//   implemented and tested the TODOs in this file.

	// After the funciton call, the context saved on the stack would've 
	//   been replaced with the context of the next task.
	
	// What we will do next is to restore the context of the next task.

	// TODO: Restore next task's fpexc and fpscr
	
	// Deallocate space previosuly allocated for saving fpexc and fpscr
	add	sp, sp, #8

	// TODO: Restore next task's d0-d15
	
	// Deallocate space previosuly allocated for saving d0-d15
	add	sp, sp, #128

	// TODO: Restore next task's r0-r14
			      	
	// Deallocate space previosuly allocated for saving r0-r14
	add	sp, sp, #60


	// Restore next task's pc and at the same time deallocte the space 
	//   previosuly allocated for saving it
	ldmia	sp!, {pc}^ // NOTE: "^" is needed because 
			   //   according to A4.1.22 LDM (3) in [1], "for an LDM 
			   //   instruction that loads the PC, this indicates that 
			   //   the SPSR of the current mode is copied to the CPSR", 
			   //   which is needed when we return from IRQ mode back to 
			   //   User/System mode.

/*
 * FIQ stub
 */
	.globl	FIQStub
FIQStub:
	sub	lr, lr, #4			/* lr: return address */
	stmfd	sp!, {r0-r3, r12, lr}		/* save r0-r3, r12 and return address */
#ifdef SAVE_VFP_REGS_ON_FIQ
	sub	sp, sp, #4			/* correct stack (number of pushs must be even) */
	vmrs	r0, fpscr			/* save VFP registers */
	stmfd	sp!, {r0}
	vstmdb	sp!, {d0-d15}
#if RASPPI >= 2 && defined (__FAST_MATH__)
	vstmdb	sp!, {d16-d31}
#endif
#endif
#if RASPPI == 1
	mov	r3, #0
	mcr	p15, 0, r3, c7, c10, 5		/* PeripheralExit() */
#endif
	ldr	r2, =FIQData
	ldr	r1, [r2]			/* get FIQData.pHandler */
	cmp	r1, #0				/* is handler set? */
	beq	1f
	ldr	r0, [r2, #4]			/* get FIQData.pParam */
	blx	r1				/* call handler (saves r4-r7) */
#if RASPPI == 1
	mov	r3, #0
	mcr	p15, 0, r3, c7, c10, 4		/* PeripheralEntry() */
#endif
#ifdef SAVE_VFP_REGS_ON_FIQ
#if RASPPI >= 2 && defined (__FAST_MATH__)
	vldmia	sp!, {d16-d31}
#endif
	vldmia	sp!, {d0-d15}			/* restore VFP registers */
	ldmfd	sp!, {r0}
	vmsr	fpscr, r0
	add	sp, sp, #4			/* correct stack */
#endif
	ldmfd	sp!, {r0-r3, r12, pc}^		/* restore registers and return */

1:
#if RASPPI == 1
	mov	r3, #0
	mcr	p15, 0, r3, c7, c10, 4		/* PeripheralEntry() */
#endif
	ldr	r1, =ARM_IC_FIQ_CONTROL		/* disable fiq (if handler is not set) */
	mov	r0, #0
	str	r0, [r1]
#if RASPPI == 1
	mov	r3, #0
	mcr	p15, 0, r3, c7, c10, 5		/* PeripheralExit() */
	mcr	p15, 0, r3, c7, c10, 4		/* PeripheralEntry() */
#endif
#ifdef SAVE_VFP_REGS_ON_FIQ
#if RASPPI >= 2 && defined (__FAST_MATH__)
	vldmia	sp!, {d16-d31}
#endif
	vldmia	sp!, {d0-d15}			/* restore VFP registers */
	ldmfd	sp!, {r0}
	vmsr	fpscr, r0
	add	sp, sp, #4			/* correct stack */
#endif
	ldmfd	sp!, {r0-r3, r12, pc}^		/* restore registers and return */

#if RASPPI >= 4

/*
 * SMC stub
 */
	.globl	SMCStub
SMCStub:
	ldr	sp, =SMCStack
	push	{lr}
	bl	SecureMonitorHandler
	pop	{lr}
	movs	pc, lr

#endif

	.data

	.align	2

	.globl	FIQData
FIQData:					/* matches TFIQData: */
	.word	0				/* pHandler */
	.word	0				/* pParam */
	.word	0				/* nFIQNumber */

	.globl	IRQReturnAddress
IRQReturnAddress:
	.word	0

#if RASPPI >= 4

	.bss

	.align	2

	.space	128
SMCStack:

#endif

/* End */

//
// task.cpp
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2015-2021  R. Stange <rsta2@o2online.de>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include <circle/sched/task.h>
#include <circle/sched/scheduler.h>
#include <circle/util.h>
#include <assert.h>
#include <circle/logger.h>
#include <circle/memory.h>
#include <circle/alloc.h>
#include <circle/user_task_exe.h>

#if AARCH == 32
	#include <circle/armv6mmu.h>
#else
	#error "64-bit ARM not supported!"
#endif

CTask::CTask (unsigned nStackSize, boolean bCreateSuspended)
:	m_State (bCreateSuspended ? TaskStateNew : TaskStateReady),
	m_bSuspended (FALSE),
	m_nStackSize (nStackSize),
	m_pStack (0),
	m_pWaitListNext (0)
{
	for (unsigned i = 0; i < TASK_USER_DATA_SLOTS; i++)
	{
		m_pUserData[i] = 0;
	}

	if (m_nStackSize != 0)
	{
		assert (m_nStackSize >= 1024);
#if AARCH == 32
		assert ((m_nStackSize & 3) == 0);
#else
		assert ((m_nStackSize & 15) == 0);
#endif
		m_pStack = new u8[m_nStackSize];
		
		assert (m_pStack != 0);

		InitializeRegs ();
	}

	m_Name.Format ("@%lp", this);

	CScheduler::Get ()->AddTask (this);
}

CTask::~CTask (void)
{
	assert (m_State == TaskStateTerminated);

	m_State = TaskStateUnknown;

	delete [] m_pStack;
	m_pStack = 0;
}

void CTask::Start (void)
{
	if (m_State == TaskStateNew)
	{
		m_State = TaskStateReady;
	}
	else
	{
		assert (m_bSuspended);
		m_bSuspended = FALSE;
	}
}

void CTask::Suspend (void)
{
	assert (m_State != TaskStateNew);
	assert (!m_bSuspended);
	m_bSuspended = TRUE;
}

void CTask::Run (void)		// dummy method which is never called
{
	assert (0);
}

void CTask::Terminate (void)
{
	m_State = TaskStateTerminated;

	m_Event.Set ();
	CScheduler::Get ()->Yield ();

	assert (0);
}

void CTask::WaitForTermination (void)
{
	// Before accessing any of our member variables
	// make sure this task object hasn't been deleted by 
	// checking it's still registered with the scheduler
	if (!CScheduler::Get()->IsValidTask (this))
	{
		return;
	}

	m_Event.Wait ();
}

void CTask::SetName (const char *pName)
{
	m_Name = pName;
}

const char *CTask::GetName (void) const
{
	return m_Name;
}

void CTask::SetUserData (void *pData, unsigned nSlot)
{
	m_pUserData[nSlot] = pData;
}

void *CTask::GetUserData (unsigned nSlot)
{
	return m_pUserData[nSlot];
}

#if AARCH == 32

void CTask::InitializeRegs (void)
{
	memset (&m_Regs, 0, sizeof m_Regs);

	m_Regs.r0 = (u32) this;		// pParam for TaskEntry()

	assert (m_pStack != 0);
	m_Regs.sp = (u32) m_pStack + m_nStackSize;

#define VFP_FPEXC_EN   (1 << 30)
	m_Regs.fpexc = VFP_FPEXC_EN;
#define VFP_FPSCR_DN   (1 << 25)       // enable Default NaN mode
	m_Regs.fpscr = VFP_FPSCR_DN;

	m_Regs.pc = (u32) &TaskEntry;

	m_Regs.cpsr = (u32)0x1F; // Kernel mode with IRQ interrupt enabled

	m_Regs.contextid = (u32)this; // Use the addr of this CTask instance as context id.
	m_Regs.ttbr0 = readTTBR0(); // Use kernel page table for this task.
}

#else

void CTask::InitializeRegs (void)
{
	memset (&m_Regs, 0, sizeof m_Regs);

	m_Regs.x0 = (u64) this;		// pParam for TaskEntry()

	assert (m_pStack != 0);
	m_Regs.sp = (u64) m_pStack + m_nStackSize;

	m_Regs.x30 = (u64) &TaskEntry;

	u64 nFPCR;
	asm volatile ("mrs %0, fpcr" : "=r" (nFPCR));
	m_Regs.fpcr = nFPCR;
}

#endif

void CTask::TaskEntry (void *pParam)
{
	CTask *pThis = (CTask *) pParam;
	assert (pThis != 0);

	pThis->Run ();

	DisableIRQs();
	pThis->m_State = TaskStateTerminated;
	pThis->m_Event.Set ();
	CScheduler::Get ()->Yield ();
	assert (0);
}


CUserModeTask::CUserModeTask(const char *exe_path)
: CTask(TASK_STACK_SIZE, TRUE)
{
	u32 *pTable = (u32*)((int)m_mem_to_contain_pTable & ~(0x4000-1)) + 0x4000;
	m_pPageTable = new CPageTable (pTable, CMemorySystem::Get()->GetKernelPageTable());
	assert (m_pPageTable != 0);

	// NOTE: In this project, the ARM virtual memory system uses pages of 1MB.
	// 	 The page table entry uses the "Section" format of [1]'s
	//	 "Table B4-2 First-level descriptor format (VMSAv6, subpages disabled)"

	// TODO: Set up ttbr0, pc, sp, cpsr properly for the user mode task.
	// Hint:
	//   - For ttbr0, read "B4.9.3 Register 2: Translation table base" to learn ttbr0's format.
	//     - You may find `m_pPageTable->GetBaseAddress();` helpful.
	//   - For pc, you need to find out what user_mode_task_exe's expected load address is
	//     and set pc to that address.
	//     - Read `sample/46-ENEE447Project4/user_mode_task/a.lst` to find out this information.
	//   - For sp, you are free to choose an address that is not unused by kernel or the exe's binary.
	//     - Google "process memory layout" to learn what a typical layout looks like.
	//   - For cpsr, you need to make sure it's user mode and IRQ interrupt is enabled.

	void *physical_page_1_baseaddr = CMemorySystem::Get()->UserModeTaskPageAllocate();	
	void *physical_page_2_baseaddr = CMemorySystem::Get()->UserModeTaskPageAllocate();	
	assert(physical_page_1_baseaddr != 0);
	assert(physical_page_2_baseaddr != 0);

	// NOTE: Normally, if we have a functioning file system, the following line 
	//       would be a file read operation which reads the binary file at 
	//	 `exe_path` from disk into memory. 
	//
	//	 Since we don't have a functioning file system, we will 
	//	 use the following hack to pretend we had read the file somehow
	//	 into the char array `user_mode_task_exe`.
	//
	//	 We also assume the binary file size is always <= 1MB so that we only
	//	 need to copy it into one physical page.
	memcpy(physical_page_1_baseaddr, user_mode_task_exe, user_mode_task_exe_len);

	u32 *pageTable = m_pPageTable->GetPageTable();
	// TODO: Set up page table entries properly for the user mode task.
	// Hint: Read `lib/pagetable.cpp` line 63-93 to see how page table entries
	//       used by the kernel are set up.

	Start();
}

CUserModeTask::~CUserModeTask(void) {
	u32 *pageTable = m_pPageTable->GetPageTable();

	// TODO: Deallocate the physical pages of this task.
	// Hint: You can do something like:
	//         void *physical_page_1_baseaddr = ...
	//         void *physical_page_2_baseaddr = ...
	//         CMemorySystem::Get()->UserModeTaskPageFree(physical_page_1_baseaddr);
	//         CMemorySystem::Get()->UserModeTaskPageFree(physical_page_2_baseaddr);
}

void CUserModeTask::Run(void) {
	// This  function should never run in a user mode task.
	assert(0);
}


int save_user_sp_and_get_kernel_sp(u32 user_sp) {
	CUserModeTask* pUserModeTask = (CUserModeTask*)(CScheduler::Get()->GetCurrentTask());
	pUserModeTask->user_sp = user_sp;
	return pUserModeTask->kernel_sp;
}

extern "C" int get_saved_user_sp() {
	CUserModeTask* pUserModeTask = (CUserModeTask*)(CScheduler::Get()->GetCurrentTask());
	return pUserModeTask->user_sp;
}



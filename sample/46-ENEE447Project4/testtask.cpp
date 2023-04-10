#include "testtask.h"

#include <circle/string.h>
#include <circle/util.h>
#include "common.h"


CUserModeTask::CUserModeTask (void)
: CTask(TASK_STACK_SIZE, FALSE, TRUE)
{

};

CUserModeTask::~CUserModeTask (void) {

};

__attribute__((naked)) int syscall_gettime() {
	asm volatile (
		"mov r7, #0x0		\n\t" // Set syscall number.
		"swi 0			\n\t" // Transition into SVC mode, call SVC handler. The <imm> field of `swi` is not used.
		"bx lr			\n\t" // Function returns.
	);
}

__attribute__((naked)) void syscall_get_task_name(char *arr, size_t arr_size) {
	asm volatile (
		"mov r7, #0x1		\n\t" // Set syscall number.
		"swi 0			\n\t" // Transition into SVC mode, call SVC handler. The <imm> field of `swi` is not used.
		"bx lr			\n\t" // Function returns.
	);
}

__attribute__((naked)) void syscall_print(const char* s) {
	asm volatile (
		"mov r7, #0x2		\n\t" // Set syscall number.
		"swi 0			\n\t" // Transition into SVC mode, call SVC handler. The <imm> field of `swi` is not used.
		"bx lr			\n\t" // Function returns.
	);
}

__attribute__((naked)) void syscall_sleep(int seconds) {
	asm volatile (
		"mov r7, #0x3		\n\t" // Set syscall number.
		"swi 0			\n\t" // Transition into SVC mode, call SVC handler. The <imm> field of `swi` is not used.
		"bx lr			\n\t" // Function returns.
	);
}

void CUserModeTask::Run (void)
{
	char task_name[128] = {0};
	
	syscall_get_task_name(task_name, 128);

	for (int i = 0; i < 3; i++) {
		CString s;
		s.Format("[%d] Task %s is running.\n", syscall_gettime(), task_name);
		syscall_print(s);
		syscall_sleep(1);
	}
}

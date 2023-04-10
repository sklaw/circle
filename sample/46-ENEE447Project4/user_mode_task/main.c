#include "my_c_library.h"

int main() {
	int nCPSR;
	asm volatile ("mrs %0, cpsr" : "=r" (nCPSR));

	char task_name[128];
	char s[1024];

	for (int i = 0; i < 128; i++) task_name[i] = 0;
	for (int i = 0; i < 1024; i++) s[i] = 0;
	
	get_task_name(task_name, 128);

	for (int i = 0; i < 3; i++) {
		simple_sprintf(s, "[%x] Task %s is running. CPSR=0x%x\n", gettime(), task_name, nCPSR);
		print(s);
		sleep(1);
	}
}

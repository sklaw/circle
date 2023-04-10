#include "my_c_library.h"
#include <stdarg.h>

int simple_sprintf(char *buffer, const char *format, ...) {
	const char *hex_digits = "0123456789abcdef";
	va_list args;
	va_start(args, format);

	int buffer_idx = 0;
	for (const char *ptr = format; *ptr; ++ptr) {
		if (*ptr == '%') {
			++ptr;
			switch (*ptr) {
				case 's': {
						  const char *str = va_arg(args, const char *);
						  while (*str) {
							  buffer[buffer_idx++] = *str++;
						  }
					  } break;
				case 'x': {
						  unsigned int value = va_arg(args, unsigned int);
						  for (int i = 7; i >= 0; i--) {
							  buffer[buffer_idx++] = hex_digits[(value >> (4*i)) & 0xF];
						  }
					  } break;
				default:
					  buffer[buffer_idx++] = '%';
					  buffer[buffer_idx++] = *ptr;
			}
		} else {
			buffer[buffer_idx++] = *ptr;
		}
	}

	buffer[buffer_idx] = '\0';
	va_end(args);

	return buffer_idx;
}



__attribute__((naked)) int gettime() {
	asm volatile (
			"mov r7, #0x0		\n\t" // Set syscall number.
			"swi 0			\n\t" // Transition into SVC mode, call SVC handler. The <imm> field of `swi` is not used.
			"bx lr			\n\t" // Function returns.
		     );
}

__attribute__((naked)) void get_task_name(char *arr, int arr_size) {
	asm volatile (
			"mov r7, #0x1		\n\t" // Set syscall number.
			"swi 0			\n\t" // Transition into SVC mode, call SVC handler. The <imm> field of `swi` is not used.
			"bx lr			\n\t" // Function returns.
		     );
}

__attribute__((naked)) void print(const char* s) {
	asm volatile (
			"mov r7, #0x2		\n\t" // Set syscall number.
			"swi 0			\n\t" // Transition into SVC mode, call SVC handler. The <imm> field of `swi` is not used.
			"bx lr			\n\t" // Function returns.
		     );
}

__attribute__((naked)) void sleep(int seconds) {
	asm volatile (
			"mov r7, #0x3		\n\t" // Set syscall number.
			"swi 0			\n\t" // Transition into SVC mode, call SVC handler. The <imm> field of `swi` is not used.
			"bx lr			\n\t" // Function returns.
		     );
}

__attribute__((naked)) void exit() {
	asm volatile (
			"mov r7, #0x4		\n\t" // Set syscall number.
			"swi 0			\n\t" // Transition into SVC mode, call SVC handler. The <imm> field of `swi` is not used.
			"bx lr			\n\t" // Function returns.
		     );
}

extern int main();
void __my_libc_start_main() {
	main();
	exit();

	print("ERROR from __my_libc_start_main: This point should not be reached because exit() should've terminated the task already.");
	while (1) {
	}
}

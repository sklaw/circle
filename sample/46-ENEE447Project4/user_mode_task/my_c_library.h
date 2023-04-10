#ifndef MY_C_LIBRARY
#define MY_C_LIBRARY

int simple_sprintf(char *buffer, const char *format, ...);
int gettime();
void get_task_name(char *arr, int arr_size);
void print(const char* s);
void sleep(int seconds);

#endif

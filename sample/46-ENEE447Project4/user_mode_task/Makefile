CCFLAGS = -ffreestanding -ffunction-sections
LDFLAGS = -nostdlib 

user_task_exe.h: a.bin
	xxd -n user_mode_task_exe -i a.bin > ../../../include/circle/user_task_exe.h

a.bin: a.elf
	arm-none-eabi-objcopy -O binary $< $@ 
	arm-none-eabi-objdump -D -d $< > a.lst

a.elf: main.o libmyclib.a
	arm-none-eabi-ld $(LDFLAGS) -T memmap.ld main.o -L. -lmyclib -o $@ -Ttext=0x80000000

libmyclib.a: my_c_library.o 
	arm-none-eabi-ar rcs $@ $<

main.o: main.c
	arm-none-eabi-gcc $(CCFLAGS) -c -o $@ $<

my_c_library.o: my_c_library.c my_c_library.h
	arm-none-eabi-gcc $(CCFLAGS) -c -o $@ $<


.PHONY: clean
clean:
	rm -f *.o *.a *.elf *.bin *.lst
	rm -f ../../../include/circle/user_task_exe.h	

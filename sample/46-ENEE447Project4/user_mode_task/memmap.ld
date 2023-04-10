SECTIONS
{

	.text : {
		*(.text.__my_libc_start_main);
		*(.text*);
	}
	.rodata : {
		*(.rodata*)
	}
	.data : {
		*(.data*)
	}
	.bss : {
		*(.bss*)
	}

}

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

ENTRY(vectors_start)

SECTIONS
{
     /* Constants used within this command: */

    __ld_Vectors_Size = 16 * 4;   /* Vectors at the very beginning of the memory (rounded up to 16) */
    __ld_Init_Addr = 0x00003000;     /* Qemu starts execution at this address */
    __ld_Svc_Stack_Size = 0x1000 - __ld_Vectors_Size; /* Very generous size of the Supervisor mode's stack (4 kB) */
    __ld_Irq_Stack_size = 0x1000; /* Very generous size of the IRQ mode's stack (4 kB) */
    __ld_Pagetable_Size = 16K;         /* Total capacity of RAM */


    . = __ld_Vectors_Size;        /* Move the pointer after the "reserved" area for exception vectors */
    . = ALIGN(16);                /* Align it to a multiple of 16; just in case... */

    . = . + __ld_Svc_Stack_Size;  /* Allocate memory for Supervisor mode's stack */
    svc_stack_top = .;            /* Initial stack pointer for the Supervisor mode */

    . = . + __ld_Irq_Stack_size; /* Allocate memory for IRQ mode's stack */
    irq_stack_top = .;           /* Initial stack pointer for the IRQ mode */

    /* Approx. 50 kB remains for the System mode's stack: */
    . = __ld_Init_Addr - 4;      /* Allocate memory for System mode's stack */
    stack_top = .;               /* It starts just in front of the startup address */

    . = ALIGN(16K);          /* Qemu will boot from this address */

    /* For MMU page table */
    .pagetable :
    {
        __pagetable_begin = .;
        *(.pagetable)
        . = __ld_Pagetable_Size;
        __pagetable_end = .;
    }

    .init :
    {
        __init_begin = .;
        *(.init)                 /* ARM exception vectors must be placed to startup address */
        __init_end = .;
    }

    /* The other sections follow: */
    .text :
    {
        __text_begin = .;
        *(.text)
        __text_end = .;
    }

    /* followed by other sections... */
    .rodata :
    {
        __rodata_begin = .;
        *(.rodata)
        __rodata_end = .;
    }

    .data :
    {
        __data_begin = .;
        *(.data)
        __data_end = .;
    }

    .bss :
    {
        __bss_begin = .;
        *(.bss)
	*(COMMON)
        __bss_end = .;
    }
    . = ALIGN(32);                  /* The section size is aligned to the 4-byte boundary */

    __ld_FootPrint_End = .;        /* A convenience symbol to determine the actual memory footprint */

    /* The remainder of RAM is reserved for the heap (if malloc is ever used) */
}

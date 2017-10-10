/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>



###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)                                                 \
    .globl name;            /* define global symbol for 'name' */              \
    .type name, @function;  /* symbol type is function */                      \
    .align 2;               /* align function definition */                    \
    name:                   /* function starts here */                         \
    pushl $(num);                                                              \
    jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)                                            \
    .globl name;                                                               \
    .type name, @function;                                                     \
    .align 2;                                                                  \
    name:                                                                      \
    pushl $0;                                                                  \
    pushl $(num);                                                              \
    jmp _alltraps
.data
	intr_arg:
		.long 2
.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */

TRAPHANDLER_NOEC(timer_intr, IRQ_OFFSET + IRQ_TIMER);
TRAPHANDLER_NOEC(kebd_intr, IRQ_OFFSET + IRQ_KBD);

TRAPHANDLER_NOEC(devide_zero_intr, 0);
TRAPHANDLER_NOEC(debug_exception_intr, 1);
TRAPHANDLER_NOEC(non_maskable_intr, 2);
TRAPHANDLER_NOEC(break_point_intr, 3);
TRAPHANDLER_NOEC(overflow_intr, 4);
TRAPHANDLER_NOEC(bound_range_exceeded_intr, 5);
TRAPHANDLER_NOEC(invalid_opcode_intr, 6);
TRAPHANDLER_NOEC(device_not_avail_intr, 7);
TRAPHANDLER(double_fault_intr, 8);
TRAPHANDLER_NOEC(correspondence_segment_overrun_intr, 9);
TRAPHANDLER(invalid_tss_intr, 10);
TRAPHANDLER(segment_not_present_intr, 11);
TRAPHANDLER(stack_segment_fault_intr, 12);
TRAPHANDLER(general_protection_intr, 13);
TRAPHANDLER(page_fault_intr, 14);
TRAPHANDLER_NOEC(unknown_intr, 15);
TRAPHANDLER_NOEC(fpu_excpt_intr, 16);
TRAPHANDLER(aligment_check_intr, 17);
TRAPHANDLER_NOEC(machine_check_intr, 18);
TRAPHANDLER_NOEC(smid_excpt_intr, 19);

TRAPHANDLER_NOEC(system_call_intr, 48);


/*
 * Lab 3: Your code here for _alltraps
 */
_alltraps:
	pushl %ds;
	pushl %es;
	pushal;
	pushl $GD_KD;
	pushl $GD_KD;
	popl %ds;
	popl %es;
	pushl %esp;
	call trap;

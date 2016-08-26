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

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */


/* from Dune: .if i <> 8 && (i <= 9 || i >= 15) && i <> 17 (then push 0) */

TRAPHANDLER_NOEC(_intr_divide,   T_DIVIDE)
TRAPHANDLER_NOEC(_intr_debug,    T_DEBUG)
TRAPHANDLER_NOEC(_intr_nmi,      T_NMI)
TRAPHANDLER_NOEC(_intr_brkpt,    T_BRKPT)
TRAPHANDLER_NOEC(_intr_oflow,    T_OFLOW)
TRAPHANDLER_NOEC(_intr_bound,    T_BOUND)
TRAPHANDLER_NOEC(_intr_illop,    T_ILLOP)
TRAPHANDLER_NOEC(_intr_device,   T_DEVICE)
TRAPHANDLER(     _intr_dblflt,   T_DBLFLT)
/* T_COPROC */
TRAPHANDLER(     _intr_tss,      T_TSS)
TRAPHANDLER(     _intr_segnp,    T_SEGNP)
TRAPHANDLER(     _intr_stack,    T_STACK)
TRAPHANDLER(     _intr_gpflt,    T_GPFLT)
TRAPHANDLER(     _intr_pgflt,    T_PGFLT)
/* T_RES */
TRAPHANDLER_NOEC(_intr_fperr,    T_FPERR)
TRAPHANDLER(     _intr_align,    T_ALIGN)
TRAPHANDLER_NOEC(_intr_mchk,     T_MCHK)
TRAPHANDLER_NOEC(_intr_simderr,  T_SIMDERR)

TRAPHANDLER_NOEC(_intr_syscall,  T_SYSCALL)


/*
 * Lab 3: Your code here for _alltraps
 */
_alltraps:

/* See COPYRIGHT for copyright information. */

#ifndef JOS_KERN_TRAP_H
#define JOS_KERN_TRAP_H
#ifndef JOS_KERNEL
# error "This is a JOS kernel header; user programs should not #include it"
#endif

#include <inc/trap.h>
#include <inc/mmu.h>

/* The kernel's interrupt descriptor table */
extern struct gatedesc idt[];
extern struct pseudodesc idt_pd;

void trap_init(void);
void trap_init_percpu(void);
void print_regs(struct pushregs *regs);
void print_trapframe(struct trapframe *tf);
void page_fault_handler(struct trapframe *);
void backtrace(struct trapframe *);
void breakpoint_handler(struct trapframe *tf);

extern void devide_zero_intr();
extern void debug_exception_intr();
extern void non_maskable_intr();
extern void break_point_intr();
extern void overflow_intr();
extern void bound_range_exceeded_intr();
extern void invalid_opcode_intr();
extern void device_not_avail_intr();
extern void double_fault_intr();
extern void correspondence_segment_overrun_intr();
extern void invalid_tss_intr();
extern void segment_not_present_intr();
extern void stack_segment_fault_intr();
extern void general_protection_intr();
extern void page_fault_intr();
extern void unknown_intr();
extern void fpu_excpt_intr();
extern void aligment_check_intr();
extern void machine_check_intr();
extern void smid_excpt_intr();

extern void timer_intr();

extern void system_call_intr();


#endif /* JOS_KERN_TRAP_H */

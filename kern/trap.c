#include <inc/mmu.h>
#include <inc/x86.h>
#include <inc/assert.h>
#include <inc/string.h>
#include <inc/mm.h>

#include <kern/pmap.h>
#include <kern/trap.h>
#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/env.h>
#include <kern/syscall.h>
#include <kern/sched.h>
#include <kern/kclock.h>
#include <kern/picirq.h>
#include <kern/cpu.h>
#include <kern/spinlock.h>

static struct taskstate ts;

/*
 * For debugging, so print_trapframe can distinguish between printing a saved
 * trapframe and printing the current trapframe and print some additional
 * information in the latter case.
 */
static struct trapframe *last_tf;

/*
 * Interrupt descriptor table.  (Must be built at run time because shifted
 * function addresses can't be represented in relocation records.)
 */
struct gatedesc idt[256] = { { 0 } };
struct pseudodesc idt_pd = {
    sizeof(idt) - 1, (uint32_t) idt
};


static const char *trapname(int trapno)
{
    static const char * const excnames[] = {
        "Divide error",
        "Debug",
        "Non-Maskable Interrupt",
        "Breakpoint",
        "Overflow",
        "BOUND Range Exceeded",
        "Invalid Opcode",
        "Device Not Available",
        "Double Fault",
        "Coprocessor Segment Overrun",
        "Invalid TSS",
        "Segment Not Present",
        "Stack Fault",
        "General Protection",
        "Page Fault",
        "(unknown trap)",
        "x87 FPU Floating-Point Error",
        "Alignment Check",
        "Machine-Check",
        "SIMD Floating-Point Exception"
    };

    if (trapno < sizeof(excnames)/sizeof(excnames[0]))
        return excnames[trapno];
    if (trapno == T_SYSCALL)
        return "System call";
    if (trapno >= IRQ_OFFSET && trapno < IRQ_OFFSET + 16)
        return "Hardware Interrupt";
    return "(unknown trap)";
}


void trap_init(void)
{
    extern struct segdesc gdt[];

    /* LAB 3: Your code here. */
		SETGATE(idt[T_DIVIDE], 0, GD_KT, devide_zero_intr, 0);
		SETGATE(idt[T_DEBUG], 0, GD_KT, debug_exception_intr, 0);
		SETGATE(idt[T_NMI], 0, GD_KT, non_maskable_intr, 0);
		SETGATE(idt[T_BRKPT], 0, GD_KT, break_point_intr, 3);
		SETGATE(idt[T_OFLOW], 0, GD_KT, overflow_intr, 0);
		SETGATE(idt[T_BOUND], 0, GD_KT, bound_range_exceeded_intr, 0);
		SETGATE(idt[T_ILLOP], 0, GD_KT, invalid_opcode_intr, 0);
		SETGATE(idt[T_DEVICE], 0, GD_KT, device_not_avail_intr, 0);
		SETGATE(idt[T_DBLFLT], 0, GD_KT, double_fault_intr, 0);
		SETGATE(idt[T_TSS], 0, GD_KT, invalid_tss_intr, 0);
		SETGATE(idt[T_SEGNP], 0, GD_KT, segment_not_present_intr, 0);
		SETGATE(idt[T_STACK], 0, GD_KT, stack_segment_fault_intr, 0);
		SETGATE(idt[T_GPFLT], 0, GD_KT, general_protection_intr, 0);
		SETGATE(idt[T_PGFLT], 0, GD_KT, page_fault_intr, 0);
		SETGATE(idt[T_FPERR], 0, GD_KT, fpu_excpt_intr, 0);
		SETGATE(idt[T_ALIGN], 0, GD_KT, aligment_check_intr, 0);
		SETGATE(idt[T_MCHK], 0, GD_KT, machine_check_intr, 0);
		SETGATE(idt[T_SIMDERR], 0, GD_KT, smid_excpt_intr, 0);

		SETGATE(idt[IRQ_OFFSET + IRQ_TIMER], 0, GD_KT, timer_intr, 0);

		SETGATE(idt[T_SYSCALL], 0, GD_KT, system_call_intr, 3);
    /* Per-CPU setup */
    trap_init_percpu();
}

/* Initialize and load the per-CPU TSS and IDT. */
void trap_init_percpu(void)
{
    /*
     * The example code here sets up the Task State Segment (TSS) and the TSS
     * descriptor for CPU 0. But it is incorrect if we are running on other CPUs
     * because each CPU has its own kernel stack.
     * Fix the code so that it works for all CPUs.
     *
     * Hints:
     *   - The macro "thiscpu" always refers to the current CPU's
     *     struct cpuinfo;
     *   - The ID of the current CPU is given by cpunum() or thiscpu->cpu_id;
     *   - Use "thiscpu->cpu_ts" as the TSS for the current CPU, rather than the
     *     global "ts" variable;
     *   - Use gdt[(GD_TSS0 >> 3) + i] for CPU i's TSS descriptor;
     *   - You mapped the per-CPU kernel stacks in mem_init_mp()
     *
     * ltr sets a 'busy' flag in the TSS selector, so if you accidentally load
     * the same TSS on more than one CPU, you'll get a triple fault.  If you set
     * up an individual CPU's TSS wrong, you may not get a fault until you try
     * to return from user space on that CPU.
     *
     * LAB 6: Your code here:
     */

     thiscpu->cpu_ts.ts_esp0 = (unsigned int)percpu_kstacks[thiscpu->cpu_id];
    thiscpu->cpu_ts.ts_ss0 = GD_KD;

    /* Initialize the TSS slot of the gdt. */
    gdt[(GD_TSS0 >> 3) + thiscpu->cpu_id] = SEG16(STS_T32A, (uint32_t) (&thiscpu->cpu_ts),
                    sizeof(struct taskstate), 0);
    gdt[(GD_TSS0 >> 3) + thiscpu->cpu_id].sd_s = 0;

    /* Load the TSS selector (like other segment selectors, the bottom three
     * bits are special; we leave them 0). */
    ltr(GD_TSS0 + thiscpu->cpu_id * 0x8);

    /* Load the IDT. */
    // lidt(&idt_pd);
    //
    // /* Setup a TSS so that we get the right stack when we trap to the kernel. */
    // ts.ts_esp0 = KSTACKTOP;
    // ts.ts_ss0 = GD_KD;
    //
    // /* Initialize the TSS slot of the gdt. */
    // gdt[GD_TSS0 >> 3] = SEG16(STS_T32A, (uint32_t) (&ts),
    //                 sizeof(struct taskstate), 0);
    // gdt[GD_TSS0 >> 3].sd_s = 0;
    //
    // /* Load the TSS selector (like other segment selectors, the bottom three
    //  * bits are special; we leave them 0). */
    // ltr(GD_TSS0);

    /* Load the IDT. */
    lidt(&idt_pd);
}

void print_trapframe(struct trapframe *tf)
{
    cprintf("TRAP frame at %p from CPU %d\n", tf, cpunum());
    print_regs(&tf->tf_regs);
    cprintf("  es   0x----%04x\n", tf->tf_es);
    cprintf("  ds   0x----%04x\n", tf->tf_ds);
    cprintf("  trap 0x%08x %s\n", tf->tf_trapno, trapname(tf->tf_trapno));
    /* If this trap was a page fault that just happened (so %cr2 is meaningful),
     * print the faulting linear address. */
    if (tf == last_tf && tf->tf_trapno == T_PGFLT)
        cprintf("  cr2  0x%08x\n", rcr2());
    cprintf("  err  0x%08x", tf->tf_err);
    /* For page faults, print decoded fault error code:
     * U/K=fault occurred in user/kernel mode
     * W/R=a write/read caused the fault
     * PR=a protection violation caused the fault (NP=page not present). */
    if (tf->tf_trapno == T_PGFLT)
        cprintf(" [%s, %s, %s]\n",
            tf->tf_err & 4 ? "user" : "kernel",
            tf->tf_err & 2 ? "write" : "read",
            tf->tf_err & 1 ? "protection" : "not-present");
    else
        cprintf("\n");
    cprintf("  eip  0x%08x\n", tf->tf_eip);
    cprintf("  cs   0x----%04x\n", tf->tf_cs);
    cprintf("  flag 0x%08x\n", tf->tf_eflags);
    if ((tf->tf_cs & 3) != 0) {
        cprintf("  esp  0x%08x\n", tf->tf_esp);
        cprintf("  ss   0x----%04x\n", tf->tf_ss);
    }
}

void print_regs(struct pushregs *regs)
{
    cprintf("  edi  0x%08x\n", regs->reg_edi);
    cprintf("  esi  0x%08x\n", regs->reg_esi);
    cprintf("  ebp  0x%08x\n", regs->reg_ebp);
    cprintf("  oesp 0x%08x\n", regs->reg_oesp);
    cprintf("  ebx  0x%08x\n", regs->reg_ebx);
    cprintf("  edx  0x%08x\n", regs->reg_edx);
    cprintf("  ecx  0x%08x\n", regs->reg_ecx);
    cprintf("  eax  0x%08x\n", regs->reg_eax);
}

static void trap_dispatch(struct trapframe *tf)
{
    /* Handle processor exceptions. */
    /* LAB 3: Your code here. */

    /* LAB 4: Update to handle more interrupts and syscall */

    /*
     * Handle spurious interrupts
     * The hardware sometimes raises these because of noise on the
     * IRQ line or other reasons. We don't care.
    */
    if (tf->tf_trapno == IRQ_OFFSET + IRQ_SPURIOUS) {
        cprintf("Spurious interrupt on irq 7\n");
        print_trapframe(tf);
        return;
    }

    /*
     * Handle clock interrupts. Don't forget to acknowledge the interrupt using
     * lapic_eoi() before calling the scheduler!
     * LAB 5: Your code here.
     */

    /* Unexpected trap: The user process or the kernel has a bug. */

		if(tf->tf_trapno == T_PGFLT){
			page_fault_handler(tf);
			return;
		} else if(tf->tf_trapno == IRQ_OFFSET + IRQ_TIMER){
			lapic_eoi();
			sched_yield();
		} else if( tf->tf_trapno == T_BRKPT){
			breakpoint_handler(tf);
			return;
		} else if(tf->tf_trapno == T_SYSCALL){
			tf->tf_regs.reg_eax = syscall(tf->tf_regs.reg_eax, tf->tf_regs.reg_edx, tf->tf_regs.reg_ecx, tf->tf_regs.reg_ebx, tf->tf_regs.reg_edi, tf->tf_regs.reg_esi);
			return;
		}

		/* Unexpected trap: The user process or the kernel has a bug. */
    print_trapframe(tf);
    if (tf->tf_cs == GD_KT)
        panic("unhandled trap in kernel");
    else {
        env_destroy(curenv);
        return;
    }
}

void trap(struct trapframe *tf)
{
    /* The environment may have set DF and some versions of GCC rely on DF being
     * clear. */
    asm volatile("cld" ::: "cc");

    /* Halt the CPU if some other CPU has called panic(). */
    extern char *panicstr;
    if (panicstr)
        asm volatile("hlt");

    /* Re-acqurie the big kernel lock if we were halted in sched_yield(). */

        		// DOn't need to lock whole kernel if we are in kernel space
    		xchg(&thiscpu->cpu_status, CPU_STARTED);
        // if (xchg(&thiscpu->cpu_status, CPU_STARTED) == CPU_HALTED)
        //     lock_kernel();

    /* Check that interrupts are disabled.
     * If this assertion fails, DO NOT be tempted to fix it by inserting a "cli"
     * in the interrupt path. */
    assert(!(read_eflags() & FL_IF));

    cprintf("Incoming TRAP frame at %p\n", tf);

    if ((tf->tf_cs & 3) == 3) {
        /* Trapped from user mode. */
        /* Acquire the big kernel lock before doing any serious kernel work.
         * LAB 6: Your code here. */

        assert(curenv);

        /* Garbage collect if current enviroment is a zombie. */
        if (curenv->env_status == ENV_DYING) {
            env_free(curenv);
            curenv = NULL;
            sched_yield();
        }

        /* Copy trap frame (which is currently on the stack) into
         * 'curenv->env_tf', so that running the environment will restart at the
         * trap point. */
        curenv->env_tf = *tf;
        /* The trapframe on the stack should be ignored from here on. */
        tf = &curenv->env_tf;
    }

    /* Record that tf is the last real trapframe so print_trapframe can print
     * some additional information. */
    last_tf = tf;

    /* Dispatch based on what type of trap occurred */
    trap_dispatch(tf);

    /* If we made it to this point, then no other environment was scheduled, so
     * we should return to the current environment if doing so makes sense. */
    if (curenv && curenv->env_status == ENV_RUNNING)
        env_run(curenv);
    else
        sched_yield();
}

void breakpoint_handler(struct trapframe *tf){
	char *buf;
	monitor(tf);
	// while(1){
	// 	buf = readline("DBG> ");
	// 	if(buf && !strcmp(buf, "printframe"))
	// 		print_trapframe(tf);
	// 	if(buf && !strcmp(buf, "kill"))
	// 		env_destroy(curenv);
	// 	else if(buf && !strcmp(buf, "continue")){
	// 		//++tf->tf_eip;
	// 		cprintf("NEW EIP = %p\n", tf->tf_eip);
	// 		return;
	// 	}
	// 	else
	// 		cprintf("GDB> Unknown command for debugger\n");
	// }
}

void page_fault_handler(struct trapframe *tf)
{
    uint32_t fault_va;
		struct vma *vma;
		/* Read processor's CR2 register to find the faulting address */
	  fault_va = rcr2();

		cprintf("[%08x] user fault va %08x ip %08x\n",
				curenv->env_id, fault_va, tf->tf_eip);
		print_trapframe(tf);

		if(tf->tf_cs == GD_KT)
			panic("Kernel space page fault");

		if(fault_va >= KERNBASE){
		  env_destroy(curenv);
			panic("Trying to access kernel space from userspace");
		}

		if(!vma_map(&(curenv->env_mm), (void*)fault_va)) // found existing vma, need to map it
			return;

    /* Destroy the environment that caused the fault. */
    env_destroy(curenv);
}

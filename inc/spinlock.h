#ifndef JOS_DEF_SPINLOCK_H
#define JOS_DEF_SPINLOCK_H

#define DEBUG_SPINLOCK
/* Disable big kernel lock
 *
 * LAB 6: Comment out the following macro definition
 *        when you are ready to move to fine-grained locking.
 */
//#define USE_BIG_KERNEL_LOCK 1

/* Mutual exclusion lock. */
struct spinlock {
    unsigned locked;       /* Is the lock held? */

#ifdef DEBUG_SPINLOCK
    /* For debugging: */
    char *name;            /* Name of lock. */
    struct cpuinfo *cpu;   /* The CPU holding the lock. */
    uintptr_t pcs[10];     /* The call stack (an array of program counters) */
                           /* that locked the lock. */
#endif
};

typedef struct spinlock spinlock_t;


#endif

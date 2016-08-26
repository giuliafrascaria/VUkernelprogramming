/* implement fork from user space */

#include <inc/string.h>
#include <inc/lib.h>

/* PTE_COW marks copy-on-write page table entries.
 * It is one of the bits explicitly allocated to user processes (PTE_AVAIL). */
#define PTE_COW     0x800

envid_t fork(void)
{
    /* LAB 5: Your code here. */
    panic("fork not implemented");
}

/*
 * Challenge!
 */
int sfork(void)
{
    panic("sfork not implemented");
    return -E_INVAL;
}

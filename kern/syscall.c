/* See COPYRIGHT for copyright information. */

#include <inc/x86.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/mm.h>


#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/trap.h>
#include <kern/syscall.h>
#include <kern/console.h>
#include <kern/sched.h>


/*
 * Print a string to the system console.
 * The string is exactly 'len' characters long.
 * Destroys the environment on memory errors.
 */
static void sys_cputs(const char *s, size_t len)
{
    /* Check that the user has permission to read memory [s, s+len).
     * Destroy the environment if not. */
		 user_mem_assert(curenv, s, len, 0);
    /* LAB 3: Your code here. */

    /* Print the string supplied by the user. */
    cprintf("%.*s", len, s);
}

/*
 * Read a character from the system console without blocking.
 * Returns the character, or 0 if there is no input waiting.
 */
static int sys_cgetc(void)
{
    return cons_getc();
}

/* Returns the current environment's envid. */
static envid_t sys_getenvid(void)
{
    return curenv->env_id;
}

/*
 * Destroy a given environment (possibly the currently running environment).
 *
 * Returns 0 on success, < 0 on error.  Errors are:
 *  -E_BAD_ENV if environment envid doesn't currently exist,
 *      or the caller doesn't have permission to change envid.
 */
static int sys_env_destroy(envid_t envid)
{
    int r;
    struct env *e;

    if ((r = envid2env(envid, &e, 1)) < 0)
        return r;
    if (e == curenv)
        cprintf("[%08x] exiting gracefully\n", curenv->env_id);
    else
        cprintf("[%08x] destroying %08x\n", curenv->env_id, e->env_id);
    env_destroy(e);
    return 0;
}

/*
 * Creates a new anonymous mapping somewhere in the virtual address space.
 *
 * Supported flags:
 *     MAP_POPULATE
 *
 * Returns the address to the start of the new mapping, on success,
 * or -1 if request could not be satisfied.
 */
static void *sys_vma_create(size_t size, int perm, int flags)
{
	void* addr = find_empty_space(size, &curenv->env_mm, perm, VMA_ANON);
	if(addr == (void*)-1)
		return (void*)-1;
	addr = do_map(&curenv->env_mm, NULL, 0, addr, size, perm, VMA_ANON);
	if(flags && MAP_POPULATE)
		madvise(&curenv->env_mm, (void*)addr, size, MADV_WILLNEED);
	return addr;
}

/*
 * Unmaps the specified range of memory starting at
 * virtual address 'va', 'size' bytes long.
 */
static int sys_vma_destroy(void *va, size_t size)
{
	do_munmap(&curenv->env_mm, va, size);
	return 0;
}


static int sys_vma_madvise(unsigned int addr, size_t size, int flags){
	madvise(&curenv->env_mm, (void*)addr, size, flags);
	return 0;
}

static int sys_vma_protect(unsigned int addr, size_t size, int flags){
	return vma_protect(&curenv->env_mm, (void*)addr, size, flags);
}

/*
 * Deschedule current environment and pick a different one to run.
 */
static void sys_yield(void)
{
    sched_yield();
}

static int sys_wait(envid_t envid)
{
    /* LAB 5: Your code here */
    return -1;
}

static int sys_fork(void)
{
   return copy_env(curenv, 0);
}

/* Dispatches to the correct kernel function, passing the arguments. */
int32_t syscall(uint32_t syscallno, uint32_t a1, uint32_t a2, uint32_t a3,
        uint32_t a4, uint32_t a5)
{
    /*
     * Call the function corresponding to the 'syscallno' parameter.
     * Return any appropriate return value.
     * LAB 3: Your code here.
     */

    switch (syscallno) {
			case SYS_cputs:
				sys_cputs((char*)a1, a2);
				break;
			case SYS_cgetc:
				return sys_cgetc();
				break;

			case SYS_getenvid:
				return sys_getenvid();
				break;

			case SYS_env_destroy:
				sys_env_destroy(a1);
				break;

			case SYS_vma_create:
				return (unsigned int)sys_vma_create(a1, a2, a3);
				break;

			case SYS_vma_destroy:
				sys_vma_destroy((void*)a1, a2);
				break;

			case SYS_vma_madvise:
				sys_vma_madvise(a1, a2, a3);
				break;

			case SYS_vma_protect:
				sys_vma_protect(a1, a2, a3);
				break;

			case SYS_fork:
				return sys_fork();
				break;
		default:
        return -E_NO_SYS;
    }
		return 0;
}

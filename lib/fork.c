/* implement fork from user space */

#include <inc/string.h>
#include <inc/lib.h>

envid_t fork(void)
{
    return sys_fork();
}

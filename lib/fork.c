/* implement fork from user space */

#include <inc/string.h>
#include <inc/lib.h>

envid_t fork(void)
{
   int res = sys_fork();
	 thisenv = &((struct env*)UENVS)[ENVX(sys_getenvid())];
	 return res;
}

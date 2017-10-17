/* Entry point for kernel threads
 * Kernel thread starts here as regular env
 * then it takes arg for function fn and put it on stack
 * when job is done it takes curenv as arg and call env_destroy
 * Inspired by Linux Kernel - Almost the same code :-)
 */

.globl kern_thread_entry
.type kern_thread_entry, @function;
.align 2;
kern_thread_entry:
.data
	message:
	.string "WTF is going on here???\n"
.text
	popl %esp
	pushl %edx; // curenv
	pushl %ebx;
	call *%eax;
	addl $0x4, %esp // curenv on top of stack again
	popl %edx
	call _this_cpu_stack
	movl %eax, %esp
	pushl %edx
	call env_destroy;

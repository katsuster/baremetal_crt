/* SPDX-License-Identifier: Apache-2.0 */

#include <stddef.h>
#include <stdint.h>

#include <bmetal/bmetal.h>
#include <bmetal/arch.h>
#include <bmetal/syscall.h>
#include <bmetal/thread.h>
#include <bmetal/sys/errno.h>
#include <bmetal/sys/string.h>

void _start(void);

int k_libc_init_main_thread(struct k_thread_info *ti, int argc, char *argv[], char *envp[], char *sp_user, char *sp_intr)
{
	intptr_t *v;

	sp_user = (void *)argv;
	sp_user -= sizeof(intptr_t);
	v = (intptr_t *)sp_user;
	*v = argc;

	k_arch_set_arg(&ti->regs, K_ARCH_ARG_TYPE_1, 0);
	k_arch_set_arg(&ti->regs, K_ARCH_ARG_TYPE_STACK, (uintptr_t)sp_user);
	k_arch_set_arg(&ti->regs, K_ARCH_ARG_TYPE_STACK_INTR, (uintptr_t)sp_intr);
	k_arch_set_arg(&ti->regs, K_ARCH_ARG_TYPE_INTADDR, (uintptr_t)_start);

	return 0;
}

const k_syscall_func_t k_table_syscalls[MAX_SYSCALLS] = {
	SYSCALL_P(SYS_uname, k_sys_wrap_uname),
	SYSCALL_P(SYS_prlimit64, k_sys_wrap_prlimit64),
	SYSCALL_P(SYS_getuid, k_sys_wrap_getuid),
	SYSCALL_P(SYS_geteuid, k_sys_wrap_geteuid),
	SYSCALL_P(SYS_getgid, k_sys_wrap_getgid),
	SYSCALL_P(SYS_getegid, k_sys_wrap_getegid),
	SYSCALL_P(SYS_getpid, k_sys_wrap_getpid),
	SYSCALL_P(SYS_gettid, k_sys_wrap_gettid),
#ifdef CONFIG_64BIT
	SYSCALL_P(SYS_clock_gettime, k_sys_wrap_clock_gettime64),
	SYSCALL_P(SYS_clock_settime, k_sys_wrap_clock_settime64),
#else /* CONFIG_64BIT */
	SYSCALL_P(SYS_clock_gettime, k_sys_wrap_clock_gettime32),
	SYSCALL_P(SYS_clock_settime, k_sys_wrap_clock_settime32),
	SYSCALL_P(SYS_clock_gettime64, k_sys_wrap_clock_gettime64),
	SYSCALL_P(SYS_clock_settime64, k_sys_wrap_clock_settime64),
#endif /* CONFIG_64BIT */
	SYSCALL_P(SYS_gettimeofday, k_sys_wrap_gettimeofday),
	SYSCALL_P(SYS_settimeofday, k_sys_wrap_settimeofday),
	SYSCALL_P(SYS_getrandom, k_sys_wrap_getrandom),
	SYSCALL_P(SYS_openat, k_sys_wrap_openat),
	SYSCALL_P(SYS_close, k_sys_wrap_close),
	SYSCALL_P(SYS_ioctl, k_sys_wrap_ioctl),
	SYSCALL_P(SYS_read, k_sys_wrap_read),
	SYSCALL_P(SYS_write, k_sys_wrap_write),
	SYSCALL_P(SYS_writev, k_sys_wrap_writev),
	SYSCALL_P(SYS_brk, k_sys_wrap_brk),
	SYSCALL_P(SYS_mmap, k_sys_wrap_mmap),
	SYSCALL_P(SYS_munmap, k_sys_wrap_munmap),
	SYSCALL_P(SYS_madvise, k_sys_wrap_madvise),
	SYSCALL_P(SYS_mprotect, k_sys_wrap_mprotect),
	SYSCALL_P(SYS_clone, k_sys_wrap_clone),
#ifdef CONFIG_64BIT
	SYSCALL_P(SYS_futex, k_sys_wrap_futex64),
#else /* CONFIG_64BIT */
	SYSCALL_P(SYS_futex, k_sys_wrap_futex32),
	SYSCALL_P(SYS_futex_time64, k_sys_wrap_futex64),
#endif /* CONFIG_64BIT */
	SYSCALL_P(SYS_set_robust_list, k_sys_wrap_set_robust_list),
	SYSCALL_P(SYS_set_tid_address, k_sys_wrap_set_tid_address),
	SYSCALL_P(SYS_exit_group, k_sys_wrap_exit_group),
	SYSCALL_P(SYS_exit, k_sys_wrap_exit),
	SYSCALL_P(SYS_reboot, k_sys_wrap_reboot),
	SYSCALL_P(SYS_clone3, k_sys_wrap_clone3),

	/* system dependent */
	SYSCALL_P(SYS_context_switch, k_sys_context_switch),
};

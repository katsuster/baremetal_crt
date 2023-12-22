/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_SYSCALL_H_
#define BAREMETAL_CRT_SYSCALL_H_

#if !defined(__ASSEMBLER__)
#include <stddef.h>
#include <stdint.h>
#endif /* !__ASSEMBLER__ */

#include <bmetal/bmetal.h>
#include <bmetal/syscall_num.h>
#include <bmetal/sys/resource.h>
#include <bmetal/sys/sched.h>
#include <bmetal/sys/poll.h>
#include <bmetal/sys/time.h>
#include <bmetal/sys/uio.h>

#define SYSCALL_P(num, func)    [(num)] = (k_syscall_func_t)(func)

#define SYSCALL_WRAP0(name) \
	intptr_t k_sys_##name(void); \
	static inline intptr_t k_sys_wrap_##name(void) \
	{ \
		return k_sys_##name(); \
	}

#define SYSCALL_WRAP1(name, t1, a1) \
	intptr_t k_sys_##name(t1 a1); \
	static inline intptr_t k_sys_wrap_##name(intptr_t a1) \
	{ \
		return k_sys_##name((t1)a1); \
	}

#define SYSCALL_WRAP2(name, t1, a1, t2, a2) \
	intptr_t k_sys_##name(t1 a1, t2 a2); \
	static inline intptr_t k_sys_wrap_##name(intptr_t a1, intptr_t a2) \
	{ \
		return k_sys_##name((t1)a1, (t2)a2); \
	}

#define SYSCALL_WRAP3(name, t1, a1, t2, a2, t3, a3) \
	intptr_t k_sys_##name(t1 a1, t2 a2, t3 a3); \
	static inline intptr_t k_sys_wrap_##name(intptr_t a1, intptr_t a2, intptr_t a3) \
	{ \
		return k_sys_##name((t1)a1, (t2)a2, (t3)a3); \
	}

#define SYSCALL_WRAP4(name, t1, a1, t2, a2, t3, a3, t4, a4) \
	intptr_t k_sys_##name(t1 a1, t2 a2, t3 a3, t4 a4); \
	static inline intptr_t k_sys_wrap_##name(intptr_t a1, intptr_t a2, intptr_t a3, intptr_t a4) \
	{ \
		return k_sys_##name((t1)a1, (t2)a2, (t3)a3, (t4)a4); \
	}

#define SYSCALL_WRAP5(name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5) \
	intptr_t k_sys_##name(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5); \
	static inline intptr_t k_sys_wrap_##name(intptr_t a1, intptr_t a2, intptr_t a3, intptr_t a4, intptr_t a5) \
	{ \
		return k_sys_##name((t1)a1, (t2)a2, (t3)a3, (t4)a4, (t5)a5); \
	}

#define SYSCALL_WRAP6(name, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6) \
	intptr_t k_sys_##name(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6); \
	static inline intptr_t k_sys_wrap_##name(intptr_t a1, intptr_t a2, intptr_t a3, intptr_t a4, intptr_t a5, intptr_t a6) \
	{ \
		return k_sys_##name((t1)a1, (t2)a2, (t3)a3, (t4)a4, (t5)a5, (t6)a6); \
	}

#define __NEW_UTS_LEN 64

#if !defined(__ASSEMBLER__)

struct new_utsname {
	char sysname[__NEW_UTS_LEN + 1];
	char nodename[__NEW_UTS_LEN + 1];
	char release[__NEW_UTS_LEN + 1];
	char version[__NEW_UTS_LEN + 1];
	char machine[__NEW_UTS_LEN + 1];
	char domainname[__NEW_UTS_LEN + 1];
};

typedef intptr_t (*k_syscall_func_t)(intptr_t no, intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f);

intptr_t k_sys_unknown(intptr_t number, intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f);

SYSCALL_WRAP1(uname, struct new_utsname *, name);
SYSCALL_WRAP4(prlimit64, pid_t, pid, int, resource, const struct rlimit64 *, new_lim, struct rlimit64 *, old_lim);
SYSCALL_WRAP0(getuid);
SYSCALL_WRAP0(geteuid);
SYSCALL_WRAP0(getgid);
SYSCALL_WRAP0(getegid);
SYSCALL_WRAP0(getpid);
SYSCALL_WRAP0(gettid);
SYSCALL_WRAP2(clock_gettime32, clockid_t, clock_id, struct timespec32 *, tp);
SYSCALL_WRAP2(clock_settime32, clockid_t, clock_id, const struct timespec32 *, tp);
SYSCALL_WRAP2(clock_gettime64, clockid_t, clock_id, struct timespec64 *, tp);
SYSCALL_WRAP2(clock_settime64, clockid_t, clock_id, const struct timespec64 *, tp);
SYSCALL_WRAP2(gettimeofday, struct timeval *, tp, void *, tzp);
SYSCALL_WRAP2(settimeofday, const struct timeval *, tp, const void *, tzp);
SYSCALL_WRAP3(getrandom, void *, buf, size_t, buflen, unsigned int, flags);
SYSCALL_WRAP4(openat, int, dirfd, const char *, pathname, int, flags, mode_t, mode);
SYSCALL_WRAP1(close, int, fd);
SYSCALL_WRAP3(ioctl, unsigned int, fd, unsigned int, cmd, unsigned long, arg);
SYSCALL_WRAP3(poll, struct pollfd *, fds, unsigned int, nfds, int, timeout);
SYSCALL_WRAP3(read, int, fd, void *, buf, size_t, count);
SYSCALL_WRAP3(write, int, fd, const void *, buf, size_t, count);
SYSCALL_WRAP3(writev, int, fd, const struct iovec *, iov, int, iovcnt);
void *__brk_area_start(void);
void *__brk_area_end(void);
SYSCALL_WRAP1(brk, void *, addr);
void *__heap_area_start(void);
void *__heap_area_end(void);
SYSCALL_WRAP6(mmap, void *, addr, size_t, length, int, prot, int, flags, int, fd, off_t, offset);
SYSCALL_WRAP2(munmap, void *, addr, size_t, length);
SYSCALL_WRAP3(madvise, void *, addr, size_t, length, int, advice);
SYSCALL_WRAP3(mprotect, void *, addr, size_t, length, int, prot);
#if defined(CONFIG_RISCV)
SYSCALL_WRAP5(clone, unsigned long, flags, void *, child_stack, void *, ptid, void *, tls, void *, ctid);
#endif
SYSCALL_WRAP6(futex32, int *, uaddr, int, op, int, val, const struct timespec32 *, timeout, int *, uaddr2, int, val3);
SYSCALL_WRAP6(futex64, int *, uaddr, int, op, int, val, const struct timespec64 *, timeout, int *, uaddr2, int, val3);
SYSCALL_WRAP2(set_robust_list, void *, head, size_t, len);
SYSCALL_WRAP1(set_tid_address, int *, tidptr);
SYSCALL_WRAP1(exit_group, int, status);
SYSCALL_WRAP1(exit, int, status);
SYSCALL_WRAP3(reboot, int, magic, int, magic2, int, cmd);
SYSCALL_WRAP2(clone3, struct clone_args *, args, size_t, size);
SYSCALL_WRAP0(context_switch);

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_SYSCALL_H_ */

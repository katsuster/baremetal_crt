/* SPDX-License-Identifier: Apache-2.0 */

#include <stdatomic.h>
#include <stdint.h>

#include <bmetal/syscall.h>
#include <bmetal/clock.h>
#include <bmetal/comm.h>
#include <bmetal/file.h>
#include <bmetal/fini.h>
#include <bmetal/init.h>
#include <bmetal/lock.h>
#include <bmetal/memory.h>
#include <bmetal/printk.h>
#include <bmetal/smp.h>
#include <bmetal/thread.h>
#include <bmetal/sys/errno.h>
#include <bmetal/sys/futex.h>
#include <bmetal/sys/inttypes.h>
#include <bmetal/sys/mman.h>
#include <bmetal/sys/random.h>
#include <bmetal/sys/reboot.h>
#include <bmetal/sys/resource.h>
#include <bmetal/sys/sched.h>
#include <bmetal/sys/string.h>

static const struct new_utsname uname = {
	.sysname    = "Linux",
	.nodename   = "",
	.release    = "5.15.0",
	.version    = "5.15.0",
	.machine    = "rv",
	.domainname = "",
};

intptr_t k_sys_unknown(intptr_t number, intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f)
{
	k_pri_info("%d: unknown syscall %"PRIdPTR"\n", k_cpu_get_current_id_phys(), number);

	return -ENOTSUP;
}

intptr_t k_sys_uname(struct new_utsname *name)
{
	k_memcpy(name, &uname, sizeof(uname));

	return 0;
}

intptr_t k_sys_prlimit64(pid_t pid, int resource, const struct rlimit64 *new_lim, struct rlimit64 *old_lim)
{
	struct rlimit64 old;

	if (new_lim) {
		k_pri_warn("prlimit64: not support to set new rlimit.\n");
		return -EPERM;
	}

	switch (resource) {
	case RLIMIT_STACK:
		old.rlim_cur = CONFIG_MAIN_STACK_SIZE;
		old.rlim_max = ~0;
		break;
	default:
		k_pri_warn("prlimit64: not support to resource %d.\n", resource);
		return -EINVAL;
	}

	if (old_lim) {
		*old_lim = old;
	}

	return 0;
}

intptr_t k_sys_getuid(void)
{
	return 0;
}

intptr_t k_sys_geteuid(void)
{
	return 0;
}

intptr_t k_sys_getgid(void)
{
	return 0;
}

intptr_t k_sys_getegid(void)
{
	return 0;
}

intptr_t k_sys_getpid(void)
{
	return k_proc_get_pid();
}

intptr_t k_sys_gettid(void)
{
	return k_thread_get_tid();
}

intptr_t k_sys_clock_gettime32(clockid_t clock_id, struct timespec32 *tp)
{
	struct timespec64 *tp64 = NULL, tmp;
	int r;

	if (tp) {
		tmp.tv_sec = tp->tv_sec;
		tmp.tv_nsec = tp->tv_nsec;
		tp64 = &tmp;
	}

	r = k_sys_clock_gettime64(clock_id, tp64);

	if (tp) {
		tp->tv_sec = tmp.tv_sec;
		tp->tv_nsec = tmp.tv_nsec;
	}

	return r;
}

intptr_t k_sys_clock_settime32(clockid_t clock_id, const struct timespec32 *tp)
{
	struct timespec64 *tp64 = NULL, tmp;

	if (tp) {
		tmp.tv_sec = tp->tv_sec;
		tmp.tv_nsec = tp->tv_nsec;
		tp64 = &tmp;
	}

	return k_sys_clock_settime64(clock_id, tp64);
}

intptr_t k_sys_clock_gettime64(clockid_t clock_id, struct timespec64 *tp)
{
	int r = 0;

	if (!tp) {
		return -EFAULT;
	}

	switch (clock_id) {
	case CLOCK_REALTIME:
		r = k_clock_get_realtime(tp);
		break;
	case CLOCK_MONOTONIC:
		r = k_clock_get_monotonic(tp);
		break;
	default:
		k_pri_warn("sys_clock_gettime: not supported clock_id:%d.\n", (int)clock_id);
		return -EINVAL;
	}

	return r;
}

intptr_t k_sys_clock_settime64(clockid_t clock_id, const struct timespec64 *tp)
{
	int r = 0;

	if (!tp) {
		return -EFAULT;
	}

	switch (clock_id) {
	case CLOCK_REALTIME:
		r = k_clock_set_realtime(tp);
		break;
	default:
		k_pri_warn("sys_clock_settime: not supported clock_id:%d.\n", (int)clock_id);
		return -EINVAL;
	}

	return r;
}

intptr_t k_sys_gettimeofday(struct timeval *tp, void *tzp)
{
	struct timespec64 tsp;
	int r;

	if (!tp) {
		return -EFAULT;
	}

	r = k_clock_get_realtime(&tsp);
	if (r) {
		return r;
	}

	tp->tv_sec = tsp.tv_sec;
	tp->tv_usec = tsp.tv_nsec / 1000L;

	return 0;
}

intptr_t k_sys_settimeofday(const struct timeval *tp, const void *tzp)
{
	struct timespec64 tsp;
	int r;

	if (!tp) {
		return -EFAULT;
	}

	tsp.tv_sec = tp->tv_sec;
	tsp.tv_nsec = tp->tv_usec * 1000L;

	r = k_clock_set_realtime(&tsp);
	if (r) {
		return r;
	}

	return 0;
}

intptr_t k_sys_getrandom(void *buf, size_t buflen, unsigned int flags)
{
	if (!buf || buflen == 0) {
		return -EINVAL;
	}

	char *p = buf;

	/* TODO: randomize */
	for (size_t i = 0; i < buflen; i++) {
		p[i] = i;
	}

	return 0;
}

intptr_t k_sys_openat(int dirfd, const char *pathname, int flags, mode_t mode)
{
	k_pri_dbg("sys_openat: %d, %s\n", dirfd, pathname);

	return -ENOTSUP;
}

intptr_t k_sys_close(int fd)
{
	struct k_file_desc *desc = k_file_get_desc(fd);
	int ret = 0;

	if (!desc) {
		return -EBADF;
	}

	if (desc->ops && desc->ops->close) {
		ret = desc->ops->close(desc);
	}

	k_file_set_desc(fd, NULL);

	return ret;
}

intptr_t k_sys_ioctl(unsigned int fd, unsigned int cmd, unsigned long arg)
{
	struct k_file_desc *desc = k_file_get_desc(fd);

	if (!desc || !desc->ops || !desc->ops->ioctl) {
		return -EBADF;
	}

	k_pri_dbg("sys_ioctl: fd:%d cmd:%d\n", fd, (int)cmd);

	return k_file_ioctl(desc, cmd, arg);
}

intptr_t k_sys_read(int fd, void *buf, size_t count)
{
	struct k_file_desc *desc = k_file_get_desc(fd);

	if (!desc || !desc->ops || !desc->ops->read) {
		return -EBADF;
	}

	k_pri_dbg("sys_read: fd:%d cnt:%d\n", fd, (int)count);

	return k_file_read(desc, buf, count);
}

intptr_t k_sys_write(int fd, const void *buf, size_t count)
{
	struct k_file_desc *desc = k_file_get_desc(fd);

	if (!desc || !desc->ops || !desc->ops->write) {
		return -EBADF;
	}

	k_pri_dbg("sys_write: fd:%d cnt:%d\n", fd, (int)count);

	return k_file_write(desc, buf, count);
}

intptr_t k_sys_writev(int fd, const struct iovec *iov, int iovcnt)
{
	struct k_file_desc *desc = k_file_get_desc(fd);
	ssize_t ret = 0, wr;

	if (!desc || !desc->ops || !desc->ops->write) {
		return -EBADF;
	}
	if (!iov) {
		return -EINVAL;
	}
	if (iovcnt < 0) {
		return -EINVAL;
	} else if (iovcnt == 0) {
		return 0;
	}

	k_spinlock_lock(&desc->lock);
	for (int i = 0; i < iovcnt; i++) {
		if (iov[i].iov_len == 0) {
			continue;
		}

		wr = k_file_write_nolock(desc, iov[i].iov_base, iov[i].iov_len);
		if (wr > 0) {
			ret += wr;
		} else if (wr < iov[i].iov_len) {
			break;
		} else {
			k_pri_warn("sys_writev: failed at %d, fd:%d len:%d.\n",
				i, fd, (int)iov[i].iov_len);
			ret = wr;
			break;
		}
	}
	k_spinlock_unlock(&desc->lock);

	return ret;
}

intptr_t k_sys_brk(void *addr)
{
	char *caddr = (char *)addr;
	char *brk_cur = k_mem_brk_get_cur();

	k_mem_brk_lock();
	if (addr == NULL) {
		k_mem_brk_unlock();
		return PTR_TO_INT(brk_cur);
	}
	if (addr < k_mem_brk_start() || k_mem_brk_end() < addr) {
		k_mem_brk_unlock();
		k_pri_info("sys_brk: addr:%p is out of bounds.\n", addr);
		return -ENOMEM;
	}

	if (caddr > brk_cur) {
		/* Expand: should zero clear by specification */
		k_memset(brk_cur, 0, caddr - brk_cur);
	} else {
		/* Shrink: zero clear for security */
		k_memset(addr, 0, brk_cur - caddr);
	}
	k_mem_brk_set_cur(addr);
	k_mem_brk_unlock();

	return PTR_TO_INT(addr);
}

intptr_t k_sys_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
	void *anon_ptr;
	int flags_req = MAP_PRIVATE | MAP_ANONYMOUS;
	size_t off_page;
	size_t len = length;

	/* Do not return same address for different area that has size == 0 */
	if (length == 0) {
		len = 4;
	}

	/* Ignore MAP_STACK */
	flags &= ~MAP_STACK;

	/*
	 * Only support anonymous mmap.
	 *   - MAP_PRIVATE && MAP_ANONYMOUS
	 *   - fd = -1
	 *   - offset = 0
	 */
	if ((flags & flags_req) != flags_req ||
	    fd != -1 || offset != 0) {
		k_pri_warn("sys_mmap: only support anonymous mmap.\n");
		return -EINVAL;
	}

	off_page = k_mem_alloc_pages(len);
	if (off_page < 0) {
		k_pri_warn("sys_mmap: no enough pages, len:%d.\n", (int)len);
		return -ENOMEM;
	}
	anon_ptr = k_mem_heap_area_start() + off_page * __PAGE_SIZE;

	k_memset(anon_ptr, 0, len);

	return PTR_TO_INT(anon_ptr);
}

intptr_t k_sys_munmap(void *addr, size_t length)
{
	if (addr < k_mem_heap_area_start() || k_mem_heap_area_end() < addr + length) {
		return -EINVAL;
	}

	k_mem_free_pages(addr, length);

	return 0;
}

intptr_t k_sys_madvise(void *addr, size_t length, int advice)
{
	switch (advice) {
	case MADV_DONTNEED:
		k_pri_info("sys_madvise: %08"PRIxPTR" - %08"PRIxPTR" do not need.\n",
			(uintptr_t)addr, (uintptr_t)addr + length);

		int r = k_mem_check_pages(addr, length);
		if (r) {
			return r;
		}

		k_memset(addr, length, 0);

		break;
	default:
		k_pri_warn("sys_madvise: advice %d is not supported.\n", advice);

		return -EINVAL;
	}

	return 0;
}

intptr_t k_sys_mprotect(void *addr, size_t length, int prot)
{
	k_pri_info("sys_mprotect: ignore mprotect.\n");

	return 0;
}

intptr_t k_sys_clone(unsigned long flags, void *child_stack, void *ptid, void *tls, void *ctid)
{
	struct k_clone_args args;

	args.flags = flags & ~CSIGNAL;
	args.exit_signal = flags & CSIGNAL;
	args.child_stack = child_stack;
	args.pidfd = ptid;
	args.ptid = ptid;
	args.tls = tls;
	args.ctid = ctid;

	return k_thread_clone(&args);
}

intptr_t k_sys_clone3(struct clone_args *cargs, size_t size)
{
	struct k_clone_args args;

	if (!cargs || size < sizeof(struct clone_args)) {
		return -EINVAL;
	}

	args.flags        = cargs->flags;
	args.pidfd        = (pid_t *)(uintptr_t)cargs->pidfd;
	args.ctid         = (pid_t *)(uintptr_t)cargs->child_tid;
	args.ptid         = (pid_t *)(uintptr_t)cargs->parent_tid;
	args.exit_signal  = cargs->exit_signal;
	args.child_stack  = (void *)(uintptr_t)cargs->stack;
	args.stack_size   = cargs->stack_size;
	args.tls          = (void *)(uintptr_t)cargs->tls;
	args.set_tid      = (pid_t *)(uintptr_t)cargs->set_tid;
	args.set_tid_size = cargs->set_tid_size;
	args.cgroup       = cargs->cgroup;

	args.child_stack += cargs->stack_size;

	return k_thread_clone(&args);
}

intptr_t k_sys_futex32(int *uaddr, int op, int val, const struct timespec32 *timeout, int *uaddr2, int val3)
{
	int cmd = op & FUTEX_MASK;
	const struct timespec64 *ts = NULL;
	struct timespec64 tmp;

	if (cmd == FUTEX_WAIT && timeout) {
		tmp.tv_sec = timeout->tv_sec;
		tmp.tv_nsec = timeout->tv_nsec;
		ts = &tmp;
	}

	return k_sys_futex64(uaddr, op, val, ts, uaddr2, val3);
}

intptr_t k_sys_futex64(int *uaddr, int op, int val, const struct timespec64 *timeout, int *uaddr2, int val3)
{
	int cmd = op & FUTEX_MASK;
	int ret = 0, r;

	if (!uaddr) {
		return -EFAULT;
	}

	switch (cmd) {
	case FUTEX_WAIT:
		if (timeout) {
			k_pri_warn("sys_futex: timeout %d.%09d is not support.\n",
				(int)timeout->tv_sec, (int)timeout->tv_nsec);
		}

		val3 = FUTEX_BITSET_ANY;
		/* fallthrough */
	case FUTEX_WAIT_BITSET:
		r = k_cpu_futex_wait(uaddr, val, val3);
		if (r) {
			if (r != -EWOULDBLOCK) {
				k_pri_warn("%d: futex err wait %p %d.\n", k_cpu_get_current_id_phys(), uaddr, val);
			}

			ret = r;
			goto err_out;
		}

		break;
	case FUTEX_WAKE:
		val3 = FUTEX_BITSET_ANY;
		/* fallthrough */
	case FUTEX_WAKE_BITSET:
		r = k_cpu_futex_wake(uaddr, val, val3);
		if (r < 0) {
			k_pri_warn("%d: futex err wake %p %d.\n", k_cpu_get_current_id_phys(), uaddr, val);

			ret = r;
			goto err_out;
		}

		ret = r;
		break;
	case FUTEX_REQUEUE:
		r = k_cpu_futex_wake(uaddr, val, FUTEX_BITSET_ANY);
		if (r < 0) {
			k_pri_warn("%d: futex err requeue %p %d.\n", k_cpu_get_current_id_phys(), uaddr, val);

			ret = r;
			goto err_out;
		}

		ret = r;
		break;
	default:
		k_pri_warn("sys_futex: cmd %d is not supported.\n", cmd);

		ret = -ENOSYS;
		goto err_out;
	}

err_out:
	return ret;
}

intptr_t k_sys_set_robust_list(void *head, size_t len)
{
	struct k_cpu_device *cpu = k_cpu_get_current();
	struct k_thread_info *ti;

	if (len == 0) {
		return -EINVAL;
	}

	ti = k_cpu_get_thread_task(cpu);
	if (!ti) {
		k_pri_err("sys_robust_list: cannot get task thread.\n");
		return -EINVAL;
	}

	ti->robust_list = head;
	ti->robust_len = len;

	return 0;
}

intptr_t k_sys_set_tid_address(int *tidptr)
{
	struct k_cpu_device *cpu = k_cpu_get_current();
	struct k_thread_info *ti;

	ti = k_cpu_get_thread_task(cpu);
	if (!ti) {
		k_pri_err("sys_set_tid_address: cannot get task thread.\n");
		return -EINVAL;
	}

	if (ti->flags & CLONE_CHILD_CLEARTID) {
		ti->ctid = tidptr;
	}

	return ti->tid;
}

intptr_t k_sys_exit_group(int status)
{
	struct k_proc_info *pi = k_proc_get_current();
	struct k_cpu_device *cur = k_cpu_get_current();
	struct k_thread_info *ti;

	k_cpu_lock(cur);

	ti = k_cpu_get_thread_task(cur);
	if (!ti) {
		k_pri_err("sys_exit_group: cannot get current task thread.\n");
		k_cpu_unlock(cur);
		return -EINVAL;
	}

	if (k_proc_get_leader(pi) != ti) {
		k_cpu_unlock(cur);
		return k_sys_exit(status);
	}

	k_cpu_unlock(cur);

	/* TODO: kill other threads in process group */

	/* Wait for children */
	for (int i = 1; i < CONFIG_NUM_CORES; i++) {
		struct k_cpu_device *cpu = k_cpu_get(i);

		if (!cpu) {
			continue;
		}

		/* TODO: avoid busy loop */
		while (k_cpu_get_thread_task(cpu)) {
			drmb();
		}
	}

	return k_sys_exit(status);
}

intptr_t k_sys_exit(int status)
{
	struct k_proc_info *pi = k_proc_get_current();
	struct k_cpu_device *cpu = k_cpu_get_current();
	struct k_thread_info *ti;
	uintptr_t v;
	int f_wake = 0, r;

	k_cpu_lock(cpu);

	ti = k_cpu_get_thread_task(cpu);
	if (!ti) {
		k_pri_err("sys_exit: cannot get task thread.\n");

		k_cpu_unlock(cpu);
		return -EINVAL;
	}

	r = k_thread_stop(ti);
	if (r) {
		k_cpu_unlock(cpu);
		return r;
	}

	r = k_thread_destroy(ti);
	if (r) {
		k_cpu_unlock(cpu);
		return r;
	}

	/* Notify to host when leader exit */
	if (k_proc_get_leader(pi) == ti) {
		k_fini_leader(status);
	} else {
		k_fini_child(status);
	}

	/* Notify to user space */
	if (ti->flags & CLONE_CHILD_CLEARTID) {
		*ti->ctid = 0;
		f_wake = 1;
	}

	r = k_thread_context_switch_nolock();
	if (r) {
		k_cpu_unlock(cpu);
		return r;
	}

	k_arch_get_arg(cpu->regs, K_ARCH_ARG_TYPE_RETVAL, &v);

	dwmb();
	k_cpu_unlock(cpu);

	if (f_wake) {
		r = k_cpu_futex_wake(ti->ctid, 1, FUTEX_BITSET_ANY);
		if (r < 0) {
			k_pri_warn("sys_exit: futex error %d but ignored.\n", r);
		}
	}

	return v;
}

intptr_t k_sys_reboot(int magic1, int magic2, int cmd)
{
	if (magic1 != REBOOT_MAGIC1 ||
	    magic2 != REBOOT_MAGIC2) {
		return -EINVAL;
	}

	switch (cmd) {
	case RB_AUTOBOOT:
		k_pri_err("Restarting system.\n");
		k_fini_reboot();
		break;
	case RB_POWER_OFF:
		k_pri_err("Power down.\n");
		k_fini_power_off();
		break;
	default:
		k_pri_warn("sys_reboot: cmd %d is not supported.\n", cmd);
		return -EINVAL;
	}

	return 0;
}

intptr_t k_sys_context_switch(void)
{
	struct k_cpu_device *cpu = k_cpu_get_current();
	uintptr_t v;
	int r;

	r = k_thread_context_switch();
	if (r) {
		/* TODO: fatal error, panic? */
		k_pri_err("sys_context_switch: failed to context_switch.\n");
		return r;
	}

	k_arch_get_arg(cpu->regs, K_ARCH_ARG_TYPE_RETVAL, &v);

	return v;
}

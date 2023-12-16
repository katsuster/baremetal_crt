/* SPDX-License-Identifier: Apache-2.0 */

#include <stdatomic.h>

#include <bmetal/driver/cpu.h>
#include <bmetal/device.h>
#include <bmetal/printk.h>
#include <bmetal/smp.h>
#include <bmetal/driver/intc.h>
#include <bmetal/sys/errno.h>

static struct k_cpu_device *cpus[CONFIG_NUM_CORES];
static int uniq_id_cpu = 1;

int k_cpu_get_id(struct k_cpu_device *cpu)
{
	return cpu->id_cpu;
}

int k_cpu_get_id_phys(struct k_cpu_device *cpu)
{
	return cpu->id_phys;
}

int k_cpu_get_running(struct k_cpu_device *cpu)
{
	return cpu->running;
}

void k_cpu_set_running(struct k_cpu_device *cpu, int r)
{
	cpu->running = r;
}


k_arch_user_regs_t *k_cpu_get_user_regs(struct k_cpu_device *cpu)
{
	return cpu->regs;
}

void k_cpu_set_user_regs(struct k_cpu_device *cpu, k_arch_user_regs_t *regs)
{
	cpu->regs = regs;
}

k_arch_user_regs_t *k_cpu_get_current_user_regs(void)
{
	return k_cpu_get_user_regs(k_cpu_get_current());
}

void k_cpu_set_current_user_regs(k_arch_user_regs_t *regs)
{
	k_cpu_set_user_regs(k_cpu_get_current(), regs);
}


int k_cpu_lock(struct k_cpu_device *cpu)
{
	return k_spinlock_lock(&cpu->lock);
}

int k_cpu_unlock(struct k_cpu_device *cpu)
{
	return k_spinlock_unlock(&cpu->lock);
}

struct k_thread_info *k_cpu_get_thread(struct k_cpu_device *cpu)
{
	return cpu->ti;
}

void k_cpu_set_thread(struct k_cpu_device *cpu, struct k_thread_info *ti)
{
	cpu->ti = ti;
}
struct k_thread_info *k_cpu_get_thread_idle(struct k_cpu_device *cpu)
{
	return cpu->ti_idle;
}

void k_cpu_set_thread_idle(struct k_cpu_device *cpu, struct k_thread_info *ti)
{
	cpu->ti_idle = ti;
}

struct k_thread_info *k_cpu_get_thread_task(struct k_cpu_device *cpu)
{
	return cpu->ti_task;
}

void k_cpu_set_thread_task(struct k_cpu_device *cpu, struct k_thread_info *ti)
{
	cpu->ti_task = ti;
}


int k_cpu_alloc_id(void)
{
	return uniq_id_cpu++;
}

struct k_cpu_device *k_cpu_get(int id)
{
	if (id < 0 || CONFIG_NUM_CORES <= id) {
		k_pri_warn("cpu_get: id:%d is out of bounds.\n", id);
		return NULL;
	}

	return cpus[id];
}

int k_cpu_set(int id, struct k_cpu_device *cpu)
{
	if (!cpu) {
		return -EINVAL;
	}
	if (id < 0 || CONFIG_NUM_CORES <= id) {
		k_pri_warn("cpu_set: id:%d is out of bounds.\n", id);
		return -EINVAL;
	}

	cpus[id] = cpu;

	return 0;
}

struct k_cpu_device *k_cpu_get_by_physical_id(int id_phys)
{
	for (int i = 0; i < CONFIG_NUM_CORES; i++) {
		if (!cpus[i]) {
			continue;
		}
		if (cpus[i]->id_phys == id_phys) {
			return cpus[i];
		}
	}
	k_pri_warn("cpu_get_by_physical_id: id_phys:%d is not found.\n", id_phys);

	return NULL;
}

int k_cpu_get_current_id_phys(void)
{
	return k_arch_get_cpu_id();
}

struct k_cpu_device *k_cpu_get_current(void)
{
	return k_cpu_get_by_physical_id(k_cpu_get_current_id_phys());
}

int k_cpu_add_device(struct k_cpu_device *cpu, struct k_bus *parent)
{
	int r;

	r = k_device_add(k_cpu_to_dev(cpu), parent);
	if (IS_ERROR(r)) {
		return r;
	}

	return 0;
}

int k_cpu_remove(struct k_cpu_device *cpu)
{
	return k_device_remove(k_cpu_to_dev(cpu));
}

int k_cpu_cache_get_line_size_i(struct k_cpu_device *cpu)
{
	return cpu->line_size_i;
}

void k_cpu_cache_set_line_size_i(struct k_cpu_device *cpu, int sz)
{
	cpu->line_size_i = sz;
}

int k_cpu_cache_get_line_size_d(struct k_cpu_device *cpu)
{
	return cpu->line_size_d;
}

void k_cpu_cache_set_line_size_d(struct k_cpu_device *cpu, int sz)
{
	cpu->line_size_d = sz;
}

int k_cpu_cache_clean_range(struct k_cpu_device *cpu, const void *start, size_t sz)
{
	const struct k_cpu_driver *drv = k_cpu_get_drv(cpu);
	int r;

	if (drv && drv->ops && drv->ops->clean_range) {
		r = drv->ops->clean_range(cpu, start, sz);
		if (r) {
			k_dev_err(k_cpu_to_dev(cpu), "clean range:%p-%p failed.\n",
				start, start + sz);
			return r;
		}
	}

	return 0;
}

int k_cpu_cache_inv_range(struct k_cpu_device *cpu, const void *start, size_t sz)
{
	const struct k_cpu_driver *drv = k_cpu_get_drv(cpu);
	int r;

	if (drv && drv->ops && drv->ops->inv_range) {
		r = drv->ops->inv_range(cpu, start, sz);
		if (r) {
			k_dev_err(k_cpu_to_dev(cpu), "invalidate range:%p-%p failed.\n",
				start, start + sz);
			return r;
		}
	}

	return 0;
}

int k_cpu_cache_flush_range(struct k_cpu_device *cpu, const void *start, size_t sz)
{
	const struct k_cpu_driver *drv = k_cpu_get_drv(cpu);
	int r;

	if (drv && drv->ops && drv->ops->flush_range) {
		r = drv->ops->flush_range(cpu, start, sz);
		if (r) {
			k_dev_err(k_cpu_to_dev(cpu), "flush range:%p-%p failed.\n",
				start, start + sz);
			return r;
		}
	}

	return 0;
}

static int k_cpu_get_handler_head(struct k_cpu_device *cpu, enum k_cpu_event event, struct k_event_handler **head)
{
	struct k_device *dev = k_cpu_to_dev(cpu);
	struct k_event_handler *h;

	if (CPU_EVENT_MAX <= event) {
		k_dev_err(dev, "CPU event %d is too large.\n", event);
		return -EINVAL;
	}

	switch (event) {
	case CPU_EVENT_ON_WAKEUP:
	case CPU_EVENT_ON_SLEEP:
	case CPU_EVENT_INTR_EXT:
	case CPU_EVENT_INTR_IPI:
	case CPU_EVENT_INTR_TIMER:
		h = &cpu->handlers[event];
		break;
	default:
		k_dev_err(dev, "CPU event %d is unknown.\n", event);
		return -EINVAL;
	}

	if (head) {
		*head = h;
	}

	return 0;
}

static int k_cpu_call_handler(struct k_cpu_device *cpu, enum k_cpu_event event)
{
	struct k_event_handler *head;
	int r;

	r = k_cpu_get_handler_head(cpu, event, &head);
	if (r) {
		return r;
	}

	if (k_event_has_next(head)) {
		k_event_handle_generic(event, head->hnd_next);
	}

	return 0;
}

int k_cpu_add_handler(struct k_cpu_device *cpu, enum k_cpu_event event, struct k_event_handler *handler)
{
	struct k_event_handler *head;
	int r;

	r = k_cpu_get_handler_head(cpu, event, &head);
	if (r) {
		return r;
	}

	handler->event = event;

	r = k_event_add_handler(head, handler);
	if (r) {
		return r;
	}

	return 0;
}

int k_cpu_remove_handler(struct k_cpu_device *cpu, enum k_cpu_event event, struct k_event_handler *handler)
{
	struct k_event_handler *head;
	int r;

	r = k_cpu_get_handler_head(cpu, event, &head);
	if (r) {
		return r;
	}

	r = k_event_remove_handler(head, handler);
	if (r) {
		return r;
	}

	return 0;
}

int k_cpu_wakeup(struct k_cpu_device *cpu)
{
	const struct k_cpu_driver *drv = k_cpu_get_drv(cpu);
	int r;

	k_cpu_set_running(cpu, 1);
	dwmb();

	if (drv && drv->ops && drv->ops->wakeup) {
		r = drv->ops->wakeup(cpu);
		if (r) {
			k_dev_err(k_cpu_to_dev(cpu), "wakeup id:%d(phys:%d) failed.\n",
				cpu->id_cpu, cpu->id_phys);
			return r;
		}
	}

	return 0;
}

int k_cpu_sleep(struct k_cpu_device *cpu)
{
	const struct k_cpu_driver *drv = k_cpu_get_drv(cpu);
	int r;

	if (drv && drv->ops && drv->ops->sleep) {
		r = drv->ops->sleep(cpu);
		if (r) {
			k_dev_err(k_cpu_to_dev(cpu), "sleep id:%d(phys:%d) failed.\n",
				cpu->id_cpu, cpu->id_phys);
			return r;
		}
	}

	k_cpu_set_running(cpu, 0);
	dwmb();

	return 0;
}

int k_cpu_wakeup_all(void)
{
	struct k_cpu_device *cpu;
	int r, res = 0;

	/* Main core is enabled */
	cpu = k_cpu_get(0);
	if (!cpu) {
		k_pri_err("cpu_wakeup_all: cannot get main core.\n");
		return -ENOMEM;
	}

	k_cpu_set_running(cpu, 1);

	/* Main core (id:0) already booted, start from 1 */
	for (int i = 1; i < CONFIG_NUM_CORES; i++) {
		cpu = k_cpu_get(i);

		if (!cpu) {
			continue;
		}

		r = k_cpu_wakeup(cpu);
		if (r) {
			res = r;
		}

		r = k_cpu_raise_ipi(cpu, NULL);
		if (r) {
			res = r;
		}
	}

	return res;
}

int k_cpu_sleep_all(void)
{
	struct k_cpu_device *cpu;
	int r, res = 0;

	/* Main core (id:0) cannot sleep, start from 1 */
	for (int i = 1; i < CONFIG_NUM_CORES; i++) {
		cpu = k_cpu_get(i);

		if (!cpu) {
			continue;
		}

		r = k_cpu_sleep(cpu);
		if (r) {
			res = r;
		}

		r = k_cpu_raise_ipi(cpu, NULL);
		if (r) {
			res = r;
		}
	}

	/* Main core is disabled */
	cpu = k_cpu_get(0);
	if (!cpu) {
		k_pri_err("cpu_sleep_all: cannot get main core.\n");
		return -ENOMEM;
	}

	k_cpu_set_running(cpu, 0);

	return res;
}

int k_cpu_on_wakeup(void)
{
	struct k_cpu_device *cpu = k_cpu_get_current();
	struct k_device *dev = k_cpu_to_dev(cpu);
	const struct k_cpu_driver *drv = k_cpu_get_drv(cpu);
	int r;

	if (drv && drv->ops && drv->ops->on_wakeup) {
		r = drv->ops->on_wakeup(cpu);
		if (r) {
			k_dev_err(dev, "failed to callback on_wakeup.\n");
			return r;
		}
	}

	r = k_cpu_call_handler(cpu, CPU_EVENT_ON_WAKEUP);
	if (r) {
		k_dev_err(dev, "failed to handle event of on_wakeup.\n");
		return r;
	}

	return 0;
}

int k_cpu_on_sleep(void)
{
	struct k_cpu_device *cpu = k_cpu_get_current();
	struct k_device *dev = k_cpu_to_dev(cpu);
	const struct k_cpu_driver *drv = k_cpu_get_drv(cpu);
	int r;

	r = k_cpu_call_handler(cpu, CPU_EVENT_ON_SLEEP);
	if (r) {
		k_dev_err(dev, "failed to handle event of on_sleep.\n");
		return r;
	}

	if (drv && drv->ops && drv->ops->on_sleep) {
		r = drv->ops->on_sleep(cpu);
		if (r) {
			k_dev_err(dev, "failed to callback on_sleep.\n");
			return r;
		}
	}

	return 0;
}

int k_cpu_wait_interrupt(void)
{
	k_arch_wait_interrupt();

	return 0;
}

int k_cpu_raise_ipi(struct k_cpu_device *dest, void *arg)
{
	struct k_cpu_device *cpu = k_cpu_get_current();
	const struct k_cpu_driver *drv = k_cpu_get_drv(cpu);
	int r;

	if (drv && drv->ops && drv->ops->raise_ipi) {
		r = drv->ops->raise_ipi(cpu, dest, arg);
		if (r) {
			k_dev_err(k_cpu_to_dev(cpu), "failed to raise IPI.\n");
			return r;
		}
	} else {
		k_dev_err(k_cpu_to_dev(cpu), "not supported to raise IPI.\n");
		return -ENOTSUP;
	}

	return 0;
}

int k_cpu_futex_wait(int *uaddr, int val, int bitset)
{
	struct k_cpu_device *cpu = k_cpu_get_current();
	int res = 0;

	if (!bitset) {
		k_dev_err(k_cpu_to_dev(cpu), "futex_wait bitmask is 0.\n");
		return -EINVAL;
	}

	k_cpu_lock(cpu);

	cpu->futex.uaddr = uaddr;
	cpu->futex.val = val;
	cpu->futex.bitset = bitset;
	cpu->futex.wakeup = 0;
	volatile int *p = uaddr;

	dwmb();
	k_cpu_unlock(cpu);

	while (!cpu->futex.wakeup) {
		if (*p != cpu->futex.val) {
			res = -EWOULDBLOCK;
			break;
		}

		k_cpu_wait_interrupt();

		/* Process pending interrupts */
		k_intr_enable_local();
		k_intr_disable_local();

		drmb();
	}

	k_cpu_lock(cpu);

	cpu->futex.uaddr = 0;
	cpu->futex.val = 0;
	cpu->futex.bitset = 0;
	cpu->futex.wakeup = 0;

	dwmb();
	k_cpu_unlock(cpu);

	return res;
}

int k_cpu_futex_wake(int *uaddr, int val, int bitset)
{
	struct k_cpu_device *cpu_cur = k_cpu_get_current();
	int r, res = 0;

	if (!bitset) {
		k_dev_err(k_cpu_to_dev(cpu_cur), "futex_wake bitmask is 0.\n");
		return -EINVAL;
	}

	for (int i = 0; i < CONFIG_NUM_CORES && res < val; i++) {
		struct k_cpu_device *cpu = k_cpu_get(i);

		if (!cpu || cpu == cpu_cur) {
			continue;
		}

		k_cpu_lock(cpu);
		drmb();

		if (cpu->futex.uaddr != uaddr || !(cpu->futex.bitset & bitset)) {
			k_cpu_unlock(cpu);
			continue;
		}

		cpu->futex.wakeup = 1;

		dwmb();
		k_cpu_unlock(cpu);

		r = k_cpu_raise_ipi(cpu, NULL);
		if (r) {
			res = r;
		}

		res++;
	}

	return res;
}

int k_cpu_get_cpu_from_config(struct k_device *dev, int index, struct k_cpu_device **cpu)
{
	const char *cpu_name;
	struct k_device *tmp;
	int r;

	r = k_device_read_conf_str(dev, "cpu", &cpu_name, index);
	if (r) {
		k_dev_err(dev, "cpu name is not found, index:%d.\n", index);
		return -EINVAL;
	}

	r = k_bus_find_device(k_bus_get_root(), cpu_name, &tmp);
	if (r == -EAGAIN) {
		return -EAGAIN;
	} else if (r) {
		k_dev_err(dev, "cpu '%s' is not found.\n", cpu_name);
		return -EINVAL;
	}

	if (cpu) {
		*cpu = k_cpu_from_dev(tmp);
	}

	return 0;
}

/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/atomic.h>
#include <bmetal/lock.h>
#include <bmetal/drivers/cpu.h>

int __spinlock_init(struct __spinlock *s)
{
	return k_arch_spinlock_init(&s->lock);
}

int __spinlock_lock(struct __spinlock *s)
{
	return k_arch_spinlock_lock(&s->lock);
}

int __spinlock_trylock(struct __spinlock *s)
{
	return k_arch_spinlock_trylock(&s->lock);
}

int __spinlock_unlock(struct __spinlock *s)
{
	return k_arch_spinlock_unlock(&s->lock);
}

int k_arch_gen_spinlock_init(k_arch_spinlock_t *lock)
{
	lock->val = 0;
	lock->id = -1;

	return 0;
}

int k_arch_gen_spinlock_lock(k_arch_spinlock_t *lock)
{
	int r = 1;

	while (r) {
		r = k_arch_gen_spinlock_trylock(lock);
	}
	lock->id = __cpu_get_current_id_phys();

	return 0;
}

int k_arch_gen_spinlock_trylock(k_arch_spinlock_t *lock)
{
	int tmp = 1, val;

	val = k_aexchange(&lock->val, tmp);

	return val;
}

int k_arch_gen_spinlock_unlock(k_arch_spinlock_t *lock)
{
	lock->id = -1;
	k_astore(&lock->val, 0);

	return 0;
}

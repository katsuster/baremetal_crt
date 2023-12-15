/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_DRIVERS_TIMER_H_
#define BAREMETAL_CRT_DRIVERS_TIMER_H_

#include <stddef.h>
#include <stdint.h>

#include <bmetal/device.h>
#include <bmetal/intr.h>
#include <bmetal/sys/errno.h>
#include <bmetal/sys/time.h>

struct k_timer_device;

struct k_timer_driver_ops {
	int (* enable)(struct k_timer_device *tm, int index);
	int (* disable)(struct k_timer_device *tm, int index);
	int (* get_freq)(struct k_timer_device *tm, int index, uint64_t *freq);
	int (* set_freq)(struct k_timer_device *tm, int index, uint64_t freq);
	int (* get_raw)(struct k_timer_device *tm, int index, uint64_t *count);
	int (* set_raw)(struct k_timer_device *tm, int index, uint64_t count);
	int (* get_trigger)(struct k_timer_device *tm, int index, struct timespec64 *tsp);
	int (* set_trigger)(struct k_timer_device *tm, int index, const struct timespec64 *tsp);
};

struct k_timer_driver {
	struct k_device_driver base;

	const struct k_timer_driver_ops *ops;
};

struct k_timer_device {
	struct k_device base;
};

struct k_timer_priv_max {
	char dummy[144];
};
typedef struct k_timer_priv_max    k_timer_priv_t;
#define CHECK_PRIV_SIZE_TIMER(typ)    CHECK_PRIV_SIZE(typ, k_timer_priv_t);

static inline const struct k_timer_driver *k_timer_get_drv(const struct k_timer_device *timer)
{
	if (!timer) {
		return NULL;
	}

	return (const struct k_timer_driver *)timer->base.drv;
}

static inline struct k_device *k_timer_to_dev(struct k_timer_device *timer)
{
	if (!timer) {
		return NULL;
	}

	return &timer->base;
}

static inline struct k_timer_device *k_timer_from_dev(struct k_device *dev)
{
	return (struct k_timer_device *)dev;
}

static inline int k_timer_add_driver(struct k_timer_driver *drv)
{
	return k_driver_add(&drv->base.base);
}

static inline int k_timer_remove_driver(struct k_timer_driver *drv)
{
	return k_driver_remove(&drv->base.base);
}

#ifdef CONFIG_TIMER

struct k_timer_device *k_timer_get_default(void);
int k_timer_set_default(struct k_timer_device *timer);

int k_timer_add_device(struct k_timer_device *timer, struct k_bus *parent);
int k_timer_remove_device(struct k_timer_device *timer);

#else /* CONFIG_TIMER */

static inline struct k_timer_device *k_timer_get_default(void)
{
	return NULL;
}

static inline struct k_timer_device *k_timer_get_system(void)
{
	return NULL;
}

static inline int k_timer_set_system(struct k_timer_device *timer)
{
	return -ENOTSUP;
}

static inline int k_timer_add_device(struct k_timer_device *timer, struct k_bus *parent)
{
	return -ENOTSUP;
}

static inline int k_timer_remove_device(struct k_timer_device *timer)
{
	return -ENOTSUP;
}

#endif /* CONFIG_TIMER */

#endif /* BAREMETAL_CRT_DRIVERS_TIMER_H_ */

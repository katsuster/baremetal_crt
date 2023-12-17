/* SPDX-License-Identifier: Apache-2.0 */

#include <stdint.h>

#include <bmetal/driver/timer.h>
#include <bmetal/device.h>
#include <bmetal/printk.h>
#include <bmetal/sys/errno.h>
#include <bmetal/sys/time.h>

static struct k_timer_device *timer_default;

struct k_timer_device *k_timer_get_default(void)
{
	return timer_default;
}

int k_timer_set_default(struct k_timer_device *timer)
{
	timer_default = timer;

	return 0;
}

int k_timer_add_device(struct k_timer_device *timer, struct k_bus *parent)
{
	struct k_device *dev = k_timer_to_dev(timer);
	uint32_t val;
	int r;

	r = k_device_add(k_timer_to_dev(timer), parent);
	if (IS_ERROR(r)) {
		return r;
	}

	r = k_device_read_conf_u32(dev, "system", &val, 0);
	if (r) {
		/* Not found, default value is 0 */
		val = 0;
	}
	if (val) {
		k_timer_set_default(timer);
	}

	return 0;
}

int k_timer_remove_device(struct k_timer_device *timer)
{
	return k_device_remove(k_timer_to_dev(timer));
}

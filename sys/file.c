/* SPDX-License-Identifier: Apache-2.0 */

#include <stddef.h>
#include <stdint.h>

#include <bmetal/init.h>
#include <bmetal/file.h>
#include <bmetal/printk.h>
#include <bmetal/thread.h>
#include <bmetal/sys/string.h>

static const struct k_file_ops file_stdio_ops = {
	.read = k_file_stdio_read,
	.write = k_file_stdio_write,
};

//TODO: tentative, use GPIO
extern const struct k_file_ops gpio_line_ops;

static struct k_file_desc fds[CONFIG_MAX_FD] = {
	/* stdin */
	[0] = {
		.ops = &file_stdio_ops,
	},

	/* stdout */
	[1] = {
		.ops = &file_stdio_ops,
	},

	/* stderr */
	[2] = {
		.ops = &file_stdio_ops,
	},

	/* TODO: tentative, GPIO ch 0 */
#ifdef CONFIG_GPIO
	[3] = {
		.ops = &gpio_line_ops,
	},
#endif
};

struct k_file_desc *k_file_get_desc(int fd)
{
	struct k_proc_info *pi = k_proc_get_current();

	if (fd < 0 || CONFIG_MAX_FD <= fd) {
		k_pri_info("k_file_get_desc: fd %d is invalid\n", fd);
		return NULL;
	}

	return pi->fdset[fd];
}

struct k_file_desc *k_file_set_desc(int fd, struct k_file_desc *desc)
{
	struct k_proc_info *pi = k_proc_get_current();
	struct k_file_desc *olddesc;

	if (fd < 0 || CONFIG_MAX_FD <= fd) {
		k_pri_info("k_file_set_desc: fd %d is invalid\n", fd);
		return NULL;
	}

	olddesc = pi->fdset[fd];
	pi->fdset[fd] = desc;

	return olddesc;
}

int k_file_ioctl_nolock(struct k_file_desc *desc, unsigned int cmd, unsigned long arg)
{
	int ret = -EBADF;

	if (desc->ops && desc->ops->ioctl) {
		ret = desc->ops->ioctl(desc, cmd, arg);
	}

	return ret;
}

int k_file_ioctl(struct k_file_desc *desc, unsigned int cmd, unsigned long arg)
{
	int ret;

	k_spinlock_lock(&desc->lock);
	ret = k_file_ioctl_nolock(desc, cmd, arg);
	k_spinlock_unlock(&desc->lock);

	return ret;
}

ssize_t k_file_read_nolock(struct k_file_desc *desc, void *buf, size_t count)
{
	ssize_t ret = 0;

	if (!buf) {
		return -EINVAL;
	}
	if (count == 0) {
		return -EINVAL;
	}

	if (desc->ops && desc->ops->read) {
		ret = desc->ops->read(desc, buf, count);
	}

	return ret;
}

ssize_t k_file_read(struct k_file_desc *desc, void *buf, size_t count)
{
	ssize_t ret;

	k_spinlock_lock(&desc->lock);
	ret = k_file_read_nolock(desc, buf, count);
	k_spinlock_unlock(&desc->lock);

	return ret;
}

ssize_t k_file_write_nolock(struct k_file_desc *desc, const void *buf, size_t count)
{
	ssize_t ret = 0;

	if (!buf) {
		return -EINVAL;
	}
	if (count == 0) {
		return 0;
	}

	if (desc->ops && desc->ops->write) {
		ret = desc->ops->write(desc, buf, count);
	}

	return ret;
}

ssize_t k_file_write(struct k_file_desc *desc, const void *buf, size_t count)
{
	ssize_t ret;

	k_spinlock_lock(&desc->lock);
	ret = k_file_write_nolock(desc, buf, count);
	k_spinlock_unlock(&desc->lock);

	return ret;
}

ssize_t k_file_stdio_read(struct k_file_desc *desc, void *buf, size_t count)
{
	return k_pri_read_stdin(buf, count);
}

ssize_t k_file_stdio_write(struct k_file_desc *desc, const void *buf, size_t count)
{
	return k_pri_write_stdout(buf, count);
}

int k_file_stdio_init(struct k_proc_info *pi)
{
	k_file_set_desc(0, &fds[0]);
	k_file_set_desc(1, &fds[1]);
	k_file_set_desc(2, &fds[2]);

	/* TODO: tentative, GPIO ch 0 */
#ifdef CONFIG_GPIO
	k_file_set_desc(3, &fds[3]);
#endif

	return 0;
}

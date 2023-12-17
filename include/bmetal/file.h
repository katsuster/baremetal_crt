/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_FILE_H_
#define BAREMETAL_CRT_FILE_H_

#include <stddef.h>

#include <bmetal/bmetal.h>
#include <bmetal/lock.h>
#include <bmetal/sys/poll.h>
#include <bmetal/sys/types.h>

#if !defined(__ASSEMBLER__)

struct k_file_desc;
struct k_proc_info;

struct k_file_ops {
	ssize_t (*read)(struct k_file_desc *desc, void *buf, size_t count);
	ssize_t (*write)(struct k_file_desc *desc, const void *buf, size_t count);
	int (*ioctl)(struct k_file_desc *desc, unsigned int cmd, unsigned long arg);
	int (*poll)(struct k_file_desc *desc, struct k_poll_info *p);
	int (*close)(struct k_file_desc *desc);
};

struct k_file_desc {
	struct k_spinlock lock;
	const struct k_file_ops *ops;

	void *priv;
};

struct k_file_desc *k_file_get_desc(int fd);
struct k_file_desc *k_file_set_desc(int fd, struct k_file_desc *desc);
int k_file_ioctl_nolock(struct k_file_desc *desc, unsigned int cmd, unsigned long arg);
int k_file_ioctl(struct k_file_desc *desc, unsigned int cmd, unsigned long arg);
ssize_t k_file_read_nolock(struct k_file_desc *desc, void *buf, size_t count);
ssize_t k_file_read(struct k_file_desc *desc, void *buf, size_t count);
ssize_t k_file_write_nolock(struct k_file_desc *desc, const void *buf, size_t count);
ssize_t k_file_write(struct k_file_desc *desc, const void *buf, size_t count);

ssize_t k_file_stdio_read(struct k_file_desc *desc, void *buf, size_t count);
ssize_t k_file_stdio_write(struct k_file_desc *desc, const void *buf, size_t count);
int k_file_stdio_init(struct k_proc_info *pi);

#endif /* !__ASSEMBLER__ */

#endif /* BAREMETAL_CRT_FILE_H_ */

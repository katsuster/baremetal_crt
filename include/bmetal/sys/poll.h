/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_SYS_POLL_H_
#define BAREMETAL_CRT_SYS_POLL_H_

#include <bmetal/bmetal.h>

struct k_poll_info {
};

#define POLLIN      0x0001
#define POLLPRI     0x0002
#define POLLOUT     0x0004
#define POLLERR     0x0008
#define POLLHUP     0x0010
#define POLLNVAL    0x0020

#endif /* BAREMETAL_CRT_SYS_POLL_H_ */

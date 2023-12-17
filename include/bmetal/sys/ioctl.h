/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_SYS_IOCTL_H_
#define BAREMETAL_CRT_SYS_IOCTL_H_

#include <bmetal/bmetal.h>

/* Same as Linux ioctl */
#define _IOC_NRBITS     8
#define _IOC_TYPEBITS   8
#define _IOC_SIZEBITS   13
#define _IOC_DIRBITS    3

#define _IOC_NRMASK     ((1 << _IOC_NRBITS) - 1)
#define _IOC_TYPEMASK   ((1 << _IOC_TYPEBITS) - 1)
#define _IOC_SIZEMASK   ((1 << _IOC_SIZEBITS) - 1)
#define _IOC_DIRMASK    ((1 << _IOC_DIRBITS) - 1)

#define _IOC_NRSHIFT    0
#define _IOC_TYPESHIFT  (_IOC_NRSHIFT + _IOC_NRBITS)
#define _IOC_SIZESHIFT  (_IOC_TYPESHIFT + _IOC_TYPEBITS)
#define _IOC_DIRSHIFT   (_IOC_SIZESHIFT + _IOC_SIZEBITS)

#define _IOC_NONE       1
#define _IOC_READ       2
#define _IOC_WRITE      4

#define _IOC(dir, type, nr, size) \
	((unsigned int) \
	 (((dir)  << _IOC_DIRSHIFT)  | \
	  ((type) << _IOC_TYPESHIFT) | \
	  ((nr)   << _IOC_NRSHIFT)   | \
	  ((size) << _IOC_SIZESHIFT)))

#define _IO(type, nr)            _IOC(_IOC_NONE, (type), (nr), 0)
#define _IOR(type, nr, size)     _IOC(_IOC_READ, (type), (nr), sizeof(size))
#define _IOW(type, nr, size)     _IOC(_IOC_WRITE, (type), (nr), sizeof(size))
#define _IOWR(type, nr, size)    _IOC(_IOC_READ | _IOC_WRITE, (type), (nr), sizeof(size))

#endif /* BAREMETAL_CRT_SYS_IOCTL_H_ */

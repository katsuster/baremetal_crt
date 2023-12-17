#include <stdio.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>

int main(int argc, char *argv[], char *envp[])
{
	int fd;
	int r, ret = 0;

	printf("%s: test gpio start\n", argv[0]);
	
	struct gpio_v2_line_values lineval;

	//TODO: get fd dynamically from gpio system
	fd = 3;

	lineval.mask = 0xffffffff;

	r = ioctl(fd, GPIO_V2_LINE_GET_VALUES_IOCTL, &lineval);
	if (r) {
		ret = -1;
	}

	if (lineval.bits == 0) {
		lineval.bits = 1;
	} else {
		lineval.bits <<= 1;
	}

	r = ioctl(fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &lineval);
	if (r) {
		ret = -1;
	}

	if (ret == 0) {
		printf("%s: SUCCESS\n", argv[0]);
	} else {
		printf("%s: FAILED\n", argv[0]);
	}

	return 0;
}

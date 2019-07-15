/* bw-global.c 
 *
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <error.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/timerfd.h>
#include <dirent.h>
#include <libconfig.h>
#include <linux/gpio.h>

#ifdef __ARM_ARCH
  #include <libpq-fe.h>
#else
  #include <postgresql/libpq-fe.h>
#endif

#include "bw-global.h"

const char *bw_cfg_file		= "/etc/bienen.cfg";
const char *bw_iio_dir		= "/sys/bus/iio/devices/";
const char *bw_iio_device	= "iio:device";

int bw_get_iio_device_idx(const char* name)
{
	int ret;
	DIR *dir;
	struct dirent *dent;
	char *fname;
	int fd, pos, idx;
	char buf[50];

	dir = opendir(bw_iio_dir);
	if (!dir) {
		fprintf(stderr, "IIO not available\n");
		return -ENODEV;
	}

	while (dent = readdir(dir), dent) {

		if (!strncmp(dent->d_name, bw_iio_device, strlen(bw_iio_device))) {
		
			if (asprintf(&fname, "%s%s/of_node/name", bw_iio_dir, dent->d_name) < 0)
				goto err_out;
		
			if ((fd = open(fname, O_RDONLY, 0)) == -1) {
				fprintf(stderr, "IIO-Name not available: %s\n", fname);
				free(fname);
				goto err_out;
			}
			free(fname);

			pos = 0;
			while ((ret = read(fd, buf + pos, sizeof(buf) - pos - 1)) > 0)
				pos += ret;

			buf[pos] = 0;
			if (!strncmp(buf, name, strlen(name))) {

				if (sscanf(dent->d_name + strlen(bw_iio_device), "%d", &idx) != 1)
					goto err_out;

				closedir(dir);
				return idx;
			}
		}
	}

err_out:
	closedir(dir);

	return -ENODEV;
}

int bw_read_value(int fd, double *val)
{
	int ret;
	int pos = 0;
	char buf[50];
	double dval;

	ret = lseek (fd, 0, SEEK_SET);

	while ((ret = read(fd, buf + pos, sizeof(buf) - pos - 1)) > 0)
		pos += ret;
	buf[pos] = 0;

	if (ret == -1)
		return -error_print("read: fd=%d\n", fd);

	if (!pos) {
		fprintf(stderr, "read: !pos\n");
		return -EINVAL;
	}

	if (sscanf(buf, "%lf", &dval) != 1) {
		fprintf(stderr, "read: !sscanf\n");
		return -EINVAL;
	}

	*val = dval;

	return pos;
}

int bw_read_value_rain(int fd, double *val)
{
	int ret;
	int pos = 0;
	char buf[50];
	int nval;

	ret = lseek (fd, 0, SEEK_SET);

	while ((ret = read(fd, buf + pos, sizeof(buf) - pos - 1)) > 0)
		pos += ret;
	buf[pos] = 0;

	if (ret == -1)
		return -error_print("read: fd=%d\n", fd);

	if (!pos) {
		fprintf(stderr, "read: !pos\n");
		return -EINVAL;
	}

	if (sscanf(buf, "%d", &nval) != 1) {
		fprintf(stderr, "read: !sscanf\n");
		return -EINVAL;
	}

	*val = ((double)nval) / 4.0;

	return pos;
}

int bw_init_button(struct bw_btn* button)
{
	int ret;
	char* fname;
	struct gpioevent_request* req = &button->req;
	struct gpiohandle_data data;

	ret = asprintf(&fname, "/dev/gpiochip%d", button->chip); 
	if (ret < 0)
		return -ENOMEM;

	button->fd = open(fname, O_RDWR, 0);
	if (button->fd == -1)
		return error_print("open(btn_fd): %s; err: %s\n", 
						fname, strerror(errno));

	req->handleflags = GPIOHANDLE_REQUEST_INPUT;
	req->eventflags = GPIOEVENT_EVENT_RISING_EDGE;
	strncpy(req->consumer_label, button->label,
						sizeof(req->consumer_label)-1);
	req->lineoffset = button->nr;

	ret = ioctl(button->fd, GPIO_GET_LINEEVENT_IOCTL, req);
	if (ret == -1)
		return error_print("ioctl(btn_fd)");

	ret = ioctl(req->fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data);
	if (ret == -1)
		return error_print("ioctl(req.fd)");

	return data.values[0];
}

int bw_ioctl_button(struct bw_btn* button)
{
	int ret;
	struct gpioevent_request* req = &button->req;
	struct gpiohandle_data data;

	ret = ioctl(req->fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data);
	if (ret == -1) {
		error_print("ioctl(req.fd)");
		return -errno;
	}

	return data.values[0];
}

int bw_get_button(struct bw_btn* button)
{
	int ret;
	struct gpioevent_request* req = &button->req;
	char buf[20];
	int val;

	val = bw_ioctl_button(button);

	ret = lseek(req->fd, 0, SEEK_SET);
	ret = read(req->fd, buf, sizeof(buf));
	if (ret == -1)
		error_print("read() buf:%s", buf);

	return val;
}


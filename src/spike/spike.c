/*
 * spike.c
 *
 * lesen von der Standardeingabe
 * entfernen von Spikes (Ausreissern)
 * ausgeben auf Standardausgabe
 *
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define LINE_LEN	200

#define SPIKE_INTERVALL	5
#define RING_SIZE	(1UL << 3)

enum ring_status {
	RS_INVAL,
	RS_OK,
	RS_SPIKE,
};

char *ring_str[] = {
	"INVAL",
	"OK",
	"SPIKE",
};

struct ring {

	unsigned int head;
	float sigma;
	float mw[RING_SIZE];
	char *buf[RING_SIZE];
	enum ring_status status[RING_SIZE];
};

int ring_find_spike(struct ring *ring)
{
	int i, min, max;
	float val_min, val_max, val_sum, val_avg;
	int start;
	int anz;

	start = ring->head - SPIKE_INTERVALL;

	val_min = ring->mw[start % RING_SIZE];
	min = start;
	val_max = ring->mw[start % RING_SIZE];
	max = start;

	anz = 0;	
	for (i = start + 1; i < ring->head; i++) {

		if (ring->mw[i % RING_SIZE] == RS_INVAL)
			continue;

		if (val_min > ring->mw[i % RING_SIZE]) {
			val_min = ring->mw[i % RING_SIZE];
			min = i;
		}

		if (val_max < ring->mw[i % RING_SIZE]) {
			val_max = ring->mw[i % RING_SIZE];
			max = i;
		}
	}

	val_sum = 0.0;

	for (i = start; i < ring->head; i++) {

		if (ring->mw[i % RING_SIZE] == RS_INVAL)
			continue;

		if ((i != min) && (i != max)) {

			val_sum += ring->mw[i % RING_SIZE];
		
			anz++;
		}
	}

	val_avg = val_sum / anz;

	for (i = start; i < ring->head; i++) {

		if (ring->mw[i % RING_SIZE] == RS_INVAL)
			continue;

		if (fabsf(ring->mw[i % RING_SIZE] - val_avg) > 3 * ring->sigma) {

			ring->status[i % RING_SIZE] = RS_SPIKE;

			// printf("spike: %f\n", ring->mw[i % RING_SIZE]);

			continue;
			// printf("sigma=%f\tval_avg=%f\tanz=%d\n", ring->sigma, val_avg, anz);
		}

		// printf("%10s\t%s", ring_str[ring->status[i % RING_SIZE]], ring->buf[i % RING_SIZE]);
		printf("%s", ring->buf[i % RING_SIZE]);
	}

	return 0;
}

int main (int argn, char* argv[], char* envp[])
{
	int ret;
	int i;
	int nOpt;
	int column = 1;
	int num;
	char *p, *q;
	float f;
	char *buf;
	char *sret;
	int fd;
	char *format;
	va_list args;
	char *sbuf;
	char *str1;
	char *saveptr1;
	struct ring *ring;
	char bufx[LINE_LEN];
	float sigma = 1.0;

	while ((nOpt = getopt(argn, argv, "k:s:")) != -1) {
		switch (nOpt) {
			case 'k':
				column = atoi(optarg);
				break;
			case 's':
				sigma = atof(optarg);
				break;
			default:
				fprintf(stderr, "usage: %s [-k <column>] [-s <sigma>]\n", argv[0]);
				return 1;
		}
	}

	//close(0);
	//fd = open("daten/2018-03-06_BME00", O_RDWR, 0);
	//if (fd == -1)
	//	error(1, errno, "open()");

	ring = (struct ring *)malloc(sizeof(struct ring));
	ring->head = 0;
	ring->sigma = sigma;	/* Standardabweichung */

	for (i = 0; i < RING_SIZE; i++) {

		ring->buf[i] = malloc(LINE_LEN);
	}

	buf = ring->buf[ring->head % RING_SIZE];

	while ((sret = fgets(buf, LINE_LEN, stdin))) {

		ring->status[ring->head % RING_SIZE] = RS_INVAL;

		if (buf[0] == '#')
			goto while_end;

		strncpy(bufx, buf, sizeof(bufx));
		for (i = 0, str1 = bufx; i < column; i++, str1 = NULL) {

			sbuf = strtok_r(str1, "\t ", &saveptr1);
			if (sbuf == NULL)
				break;

			if (*sbuf == '#') {
				sbuf = NULL;
				break;
			}
		}

		if (sbuf) {

			num = sscanf(sbuf, "%f", &f);
			if (num <= 0)
				break;

			ring->status[ring->head % RING_SIZE] = RS_OK;
			ring->mw[ring->head % RING_SIZE] = f;
		}

while_end:
		if (ring->head && !(ring->head % SPIKE_INTERVALL)) {
			ret = ring_find_spike(ring);
		}

		ring->head++;
		buf = ring->buf[ring->head % RING_SIZE];
	}

	return 0;
}


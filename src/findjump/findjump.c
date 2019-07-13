/*
 * findjump.c
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

#define AVG_INTERVAL	5
#define RING_SIZE	(1UL << 4)

enum ring_status {
	RS_INVAL,
	RS_OK,
	RS_JUMP,
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

int ring_findjump(struct ring *ring)
{
	int i;
	float val_avg_1, val_avg_2;
	int start;

	start = ring->head - 3 * AVG_INTERVAL + 1;
	val_avg_1 = 0.0;

	for (i = 0; i < AVG_INTERVAL; i++)
		val_avg_1 += ring->mw[(start + i) % RING_SIZE];

	val_avg_1 /= AVG_INTERVAL;

	start = ring->head - AVG_INTERVAL + 1;
	val_avg_2 = 0.0;

	for (i = 0; i < AVG_INTERVAL; i++)
		val_avg_2 += ring->mw[(start + i) % RING_SIZE];

	val_avg_2 /= AVG_INTERVAL;

	// printf("start: %d avg1: %f avg2: %f\n", start, val_avg_1, val_avg_2);

	if (fabs(val_avg_2 - val_avg_1) > ring->sigma) {
		return RS_JUMP;
	} else {
		return RS_OK;
	}

	return 0;
}

int main (int argn, char* argv[], char* envp[])
{
	int ret;
	int mret = 0;
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
	float sigma = 100.0;

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

/* zum Testen
 *
	close(0);
	fd = open("daten/2018-03-06_WAAGE00", O_RDWR, 0);
	if (fd == -1)
		error(1, errno, "open()");
*/

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
		if ((ring->head >= 3 * AVG_INTERVAL) && 
				!(ring->head % AVG_INTERVAL)) {

			ret = ring_findjump(ring);
			if (ret == RS_JUMP) {
				fprintf(stderr, "jump between %d and %d\n", 
						ring->head - 2 * AVG_INTERVAL,
						ring->head - AVG_INTERVAL);

				mret = 1;
			}
		}

		ring->head++;
		buf = ring->buf[ring->head % RING_SIZE];
	}

	return mret;
}


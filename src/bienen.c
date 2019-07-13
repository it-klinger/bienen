/*
 * bienen.c
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
#include <signal.h>
#include <poll.h>
#include <dirent.h>
#include <error.h>
#include <math.h>
#include <libconfig.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/timerfd.h>
#include <sys/mman.h>
#include <linux/gpio.h>

#ifdef __ARM_ARCH
  #include <libpq-fe.h>
#else
  #include <postgresql/libpq-fe.h>
#endif

#include "bw-global.h"

int bw_set_waage_fileout(struct bw_waage *waage)
{
	int ret = 0;
	time_t time0;
	struct tm tm0;
	char *fname_out;

	time(&time0);
	localtime_r (&time0, &tm0);

	if (asprintf(&fname_out, "/daten/%04d-%02d-%02d_WAAGE%02d", 
		tm0.tm_year + 1900, tm0.tm_mon + 1, tm0.tm_mday,
		waage->idx) == -1)
		error_at_line(1,errno,__FILE__,__LINE__,"asprintf()");

	if ((waage->fnameout == NULL) 
		|| (strncmp(waage->fnameout, fname_out, strlen(fname_out)))) {

		sprintf(waage->fnameout, "%s", fname_out);
	
		if (waage->fdout > 2)
			close(waage->fdout);

		waage->fdout = open(waage->fnameout, O_RDWR | O_CREAT |
								O_APPEND, 0666);
		if (waage->fdout == -1) {
			error_at_line(1,errno,__FILE__,__LINE__,"open(): %s - %s\n",
					waage->fnameout, strerror(errno));
		} else {
			printf("Datei geöffnet: %s\n", waage->fnameout);
			ret = 0xAFFE;
		}
	}

	return ret;
}

int bw_set_bme_fileout(struct bw_stand *stand)
{
	int ret = 0;
	time_t time0;
	struct tm tm0;
	char *fname_out;

	time(&time0);
	localtime_r (&time0, &tm0);

	if (asprintf(&fname_out, "/daten/%04d-%02d-%02d_BME00", 
		tm0.tm_year + 1900, tm0.tm_mon + 1, tm0.tm_mday) == -1)
		error_at_line(1,errno,__FILE__,__LINE__,"asprintf()");

	if ((stand->fbmenameout == NULL) 
		|| (strncmp(stand->fbmenameout, fname_out, strlen(fname_out)))) {

		sprintf(stand->fbmenameout, "%s", fname_out);
	
		if (stand->fdbme > 2)
			close(stand->fdbme);

		stand->fdbme = open(stand->fbmenameout, O_RDWR | O_CREAT | O_APPEND, 0666);
		if (stand->fdbme == -1) {
			error_at_line(1,errno,__FILE__,__LINE__,"open(): %s - %s\n",
					stand->fbmenameout, strerror(errno));
		} else {
			printf("Datei geöffnet: %s\n", stand->fbmenameout);
			ret = 0xAFFE;
		}
	}

	return ret;
}

int init_stand(struct bw_stand* stand)
{
	config_t cfg;
	config_setting_t *setting;
	config_setting_t *weight;
	const char *str;
	int i = 0;
	const char *title, *dtb_name, *dtb_node, *iio_chan;
	double offset, scale, tara;
	int idx, waa_id;
	int val;
	double dval;
	int fd;
	struct bw_waage *waage;

	/* Default-Werte festlegen */
	stand->nwaagen	= 0;
	stand->rain_alt = 0.0;

	config_init(&cfg);

	if (!config_read_file(&cfg, stand->cfg_file))
		error_at_line(1,errno,__FILE__,__LINE__,"%s:%d - %s\n", config_error_file(&cfg),
			config_error_line(&cfg), config_error_text(&cfg));

	if (!config_lookup_string(&cfg, "name", &str))
		error_at_line(1,errno,__FILE__,__LINE__,"No 'name' setting in configuration file.\n");

	if (!config_lookup_int(&cfg, "globals.btn_pause.gpiochip", &val))
		error_at_line(1,errno,__FILE__,__LINE__,"No 'btn_pause_chip' setting in configuration file.\n");

	stand->btn_pause.chip = val;

	if (!config_lookup_int(&cfg, "globals.btn_pause.gpionr", &val))
		error_at_line(1,errno,__FILE__,__LINE__,"No 'btn_pause_nr' setting in configuration file.\n");

	stand->btn_pause.nr = val;

	if (!config_lookup_string(&cfg, "globals.btn_pause.label", &str))
		error_at_line(1,errno,__FILE__,__LINE__,"No 'label' setting in configuration file.\n");

	strncpy(stand->btn_pause.label, str, sizeof(stand->btn_pause.label));

	if (config_lookup_int(&cfg, "globals.zyklus", &val))
		stand->zyklus = val;
	else {
		error_at_line(0,errno,__FILE__,__LINE__,"No 'zyklus' setting in configuration file; using default 60.\n");
		stand->zyklus = 60;
	}

	if (config_lookup_int(&cfg, "globals.thr_mass", &val))
		stand->thr_mass = val;
	else {
		error_at_line(0,errno,__FILE__,__LINE__,"No 'thr_mass' setting in configuration file; using"
				" default 2000 Gramm\n");
		stand->thr_mass = 2000;
	}

	if (!config_lookup_string(&cfg, "globals.logfile", &str))
		error_at_line(1,errno,__FILE__,__LINE__,"No 'logfile' setting in configuration file.\n");

	strncpy(stand->logfile, str, sizeof(stand->logfile));
	fd = open(stand->logfile, O_RDWR | O_CREAT | O_APPEND | O_SYNC, 0666);
	if (fd == -1)
		error_at_line(1,errno,__FILE__,__LINE__,"open(): logfile=%s\n", stand->logfile);

	dup2(fd, 1);
	close(fd);

	if (!config_lookup_string(&cfg, "globals.errfile", &str))
		error_at_line(1,errno,__FILE__,__LINE__,"No 'errfile' setting in configuration file.\n");

	strncpy(stand->errfile, str, sizeof(stand->errfile));
	fd = open(stand->errfile, O_RDWR | O_CREAT | O_APPEND | O_SYNC, 0666);
	if (fd == -1)
		error_at_line(1,errno,__FILE__,__LINE__,"open(): errfile=%s\n", stand->errfile);

	dup2(fd, 2);
	close(fd);

	if (!config_lookup_string(&cfg, "globals.pressure.dtb_name", &str))
		error_at_line(1,errno,__FILE__,__LINE__, "No 'globals.pressure.dtb_name' setting in configuration file.\n");

	strncpy(stand->pressure_dtb_name, str, sizeof(stand->pressure_dtb_name)-1);

	if (!config_lookup_string(&cfg, "globals.pressure.dtb_node", &str))
		error_at_line(1,errno,__FILE__,__LINE__, "No 'globals.pressure.dtb_node' setting in configuration file.\n");
		
	strncpy(stand->pressure_dtb_node, str, sizeof(stand->pressure_dtb_node)-1);

	if (!config_lookup_float(&cfg, "globals.press_offset", &dval))
		error_at_line(1,errno,__FILE__,__LINE__,"No 'press_offset' setting in configuration file\n");

	stand->press_offset = dval;

	if (!config_lookup_string(&cfg, "globals.conn_string", &str)) {
		error_at_line(0,errno,__FILE__,__LINE__, "No 'globals.conn_string' setting in configuration file.\n");
		stand->db = 0;
	} else {
		strncpy(stand->conn_string, str, sizeof(stand->conn_string)-1);
		stand->db = 1;
	}

	setting = config_lookup(&cfg, "bees.weights");
	if (setting == NULL)
		error_at_line(1,errno,__FILE__,__LINE__,"No 'bees.weights' setting in configuration file\n");

	stand->nwaagen = config_setting_length(setting);

	printf("%3s %-15s  %-10s  %-10s  %-16s  %-7s  %-11s  %-12s\n", 
			"IDX", "TITLE", "DTB-NAME", "DTB-NODE", 
			"IIO-CHAN", "OFFSET", "SCALE", "TARA");

	stand->waage = malloc(stand->nwaagen * sizeof(struct bw_waage*));

	for (i = 0; i < stand->nwaagen; i++) {

		weight = config_setting_get_elem(setting, i);

		if (!(config_setting_lookup_string(weight, "title", &title)
					&& config_setting_lookup_string(weight, "dtb_name", &dtb_name)
					&& config_setting_lookup_string(weight, "dtb_node", &dtb_node)
					&& config_setting_lookup_string(weight, "iio_chan", &iio_chan)
					&& config_setting_lookup_int(weight, "idx", &idx)
					&& config_setting_lookup_int(weight, "waa_id", &waa_id)
					&& config_setting_lookup_float(weight, "offset", &offset)
					&& config_setting_lookup_float(weight, "scale", &scale)
					&& config_setting_lookup_float(weight, "tara", &tara)))
			continue;

		stand->waage[i] = malloc(sizeof(struct bw_waage));
		waage = stand->waage[i];

		waage->idx = idx;
		waage->waa_id = waa_id;
		strncpy(waage->title, title, sizeof(waage->title));
		strncpy(waage->dtb_name, dtb_name, sizeof(waage->dtb_name));
		strncpy(waage->dtb_node, dtb_node, sizeof(waage->dtb_node));
		strncpy(waage->iio_chan, iio_chan, sizeof(waage->iio_chan));
		waage->offset = offset;
		waage->scale = scale;
		waage->tara = tara;

		printf("%2d  %4d  %-15s  %-10s  %-10s  %-15s  %8.0f  %8.6f  %8.0f\n",
				waage->idx, waage->waa_id, waage->title,
				waage->dtb_name, waage->dtb_node,
				waage->iio_chan, waage->offset,
				waage->scale, waage->tara);
	}
	putchar('\n');

	config_destroy(&cfg);

	return i;
}

int init_waagen_btn_tmr(struct bw_stand* stand)
{
	int i, j;
	int ret;
	int idx;
	char *fname;
	double val;
	struct timespec now;
	struct itimerspec new_value;
	struct bw_shm *shm;

	idx = bw_get_iio_device_idx(stand->pressure_dtb_node);
	if (idx < 0) {
		error_at_line(0,errno,__FILE__,__LINE__,"iio-device for bme280 not found: %s\n", stand->pressure_dtb_node);
		stand->fdtemp = -1;
		stand->fdhumi = -1;
		stand->fdpres = -1;
		stand->fdrain = -1;
	} else {
		asprintf(&fname, "%s%s%d/in_temp_input", bw_iio_dir, bw_iio_device, idx);
		stand->fdtemp = open(fname, O_RDONLY);
		if (stand->fdtemp == -1)
			error_at_line(0,errno,__FILE__,__LINE__,"open(fdtemp) fname=%s\n", fname);
		free (fname);

		asprintf(&fname, "%s%s%d/in_pressure_input", bw_iio_dir, bw_iio_device, idx);
		stand->fdpres = open(fname, O_RDONLY);
		if (stand->fdpres == -1)
			error_at_line(0,errno,__FILE__,__LINE__,"open(fdpres) fname=%s\n", fname);
		free (fname);

		asprintf(&fname, "%s%s%d/in_humidityrelative_input", bw_iio_dir, bw_iio_device, idx);
		stand->fdhumi = open(fname, O_RDONLY);
		if (stand->fdhumi == -1)
			error_at_line(0,errno,__FILE__,__LINE__,"open(fdhumi) fname=%s\n", fname);
		free (fname);

		asprintf(&fname, "/dev/rain0");
		stand->fdrain = open(fname, O_RDONLY);
		if (stand->fdrain == -1)
			error_at_line(0,errno,__FILE__,__LINE__,"open(fdrain) fname=%s\n", fname);
		free (fname);
	}

	for (i = 0; i < stand->nwaagen; i++) {

		idx = bw_get_iio_device_idx(stand->waage[i]->dtb_node);
		if (idx < 0) {
			error_at_line(1,errno,__FILE__,__LINE__,"iio-device for weight not found: %s\n",
					stand->waage[i]->dtb_node);
			return idx;
		}

		asprintf(&fname, "%s%s%d/%s", bw_iio_dir, bw_iio_device,
				idx, stand->waage[i]->iio_chan);
		stand->waage[i]->fd = open(fname, O_RDONLY);
		if (stand->waage[i]->fd == -1)
			error_at_line(1,errno,__FILE__,__LINE__,"open(fdhx[1]): %s\n", fname);
		free (fname);
	}

	ret = bw_init_button(&stand->btn_pause);
	if (ret < 0)
		error_at_line(1,errno,__FILE__,__LINE__,"bw_init_button(pause)");

	if (stand->zyklus) {
		stand->fdtmr = timerfd_create (CLOCK_REALTIME, 0);
		if (stand->fdtmr == -1)
			error_at_line(1,errno,__FILE__,__LINE__,"timerfd_create()\n");

		ret = clock_gettime (CLOCK_REALTIME, &now);
		now.tv_sec /= stand->zyklus;
		now.tv_sec *= stand->zyklus;
		now.tv_sec += stand->zyklus;

		new_value.it_interval.tv_sec = stand->zyklus;
		new_value.it_interval.tv_nsec = 0;
		new_value.it_value.tv_sec = now.tv_sec;
		new_value.it_value.tv_nsec = 0;

		ret = timerfd_settime(stand->fdtmr, TFD_TIMER_ABSTIME, &new_value, NULL);
		if (ret == -1) {
			error_at_line(1,errno,__FILE__,__LINE__,"timerfd_settime(): err=%s\n", strerror(errno));
			return -errno;
		}
	}

	stand->fdshm = shm_open("bw", O_CREAT | O_RDWR, 0666);
	if (stand->fdshm < 0)
		error_at_line(1,errno,__FILE__,__LINE__,"shm_open()");

	ret = ftruncate(stand->fdshm, BW_SHM_SIZE);
	if (ret < 0)
		error_at_line(1,errno,__FILE__,__LINE__,"ftruncate()");

	stand->shm = mmap(0, BW_SHM_SIZE, PROT_READ | PROT_WRITE,
						MAP_SHARED, stand->fdshm, 0);
	if (stand->shm == MAP_FAILED)
		error_at_line(1,errno,__FILE__,__LINE__,"mmap()");

	shm = stand->shm;
	shm->head.nwaagen = stand->nwaagen;
	snprintf(shm->head.snwaagen, sizeof(shm->head.snwaagen), "%d", stand->nwaagen);

	return 0;
}

int read_waagen_daten(struct bw_stand *stand)
{
	int i;
	int ret;
	double val;

	for (i = 0; i < stand->nwaagen; i++) {

		ret = bw_read_value(stand->waage[i]->fd, &val);
		if (ret < 0) {
			error_at_line(1,errno,__FILE__,__LINE__,"kein gültiger Meßwert\n");
			break;
		}

		if (ret > 0)
			stand->waage[i]->mass = val;
	}

	return 0;
}

int read_bme_daten(struct bw_stand *stand)
{
	double val;

	stand->temperature = -100.0;
	stand->pressure = -1.0;
	stand->humidity = -1.0;
	stand->rain = -1.0;

	if (stand->fdtemp > 0)
		if (bw_read_value(stand->fdtemp, &val) > 0)
			stand->temperature = val;

	if (stand->fdpres > 0)
		if (bw_read_value(stand->fdpres, &val) > 0)
			stand->pressure = val + stand->press_offset;

	if (stand->fdhumi > 0)
		if (bw_read_value(stand->fdhumi, &val) > 0)
			stand->humidity = val;

	if (stand->fdrain > 0)
		if (bw_read_value_rain(stand->fdrain, &val) > 0)
			stand->rain = val;

	return 0;
};

int write_bme_waagen_daten(struct bw_stand *stand)
{
	int i;
	int ret;
	char sline[200];
	char datetime[30];
	char ts[30];
	time_t time0;
	struct tm tm0;
	struct bw_shm *shm = stand->shm;

	time(&time0);
	localtime_r (&time0, &tm0);

	memset(ts, 0, sizeof(ts));
	sprintf(ts, "%02d.%02d.%04d %02d:%02d:%02d", 
			tm0.tm_mday, tm0.tm_mon + 1,
			tm0.tm_year + 1900, 
			tm0.tm_hour, tm0.tm_min,
			tm0.tm_sec);

	memset(datetime, 0, sizeof(datetime));
	sprintf(datetime, "%02d.%02d.%04d-%02d:%02d:%02d", 
			tm0.tm_mday, tm0.tm_mon + 1,
			tm0.tm_year + 1900, 
			tm0.tm_hour, tm0.tm_min,
			tm0.tm_sec);

	memcpy(shm->head.ts, datetime, sizeof(shm->head.ts));

	ret = bw_set_bme_fileout(stand);
	if (ret == 0xAFFE) {
		// neues File aufgemacht
		ret = lseek(stand->fdbme, 0, SEEK_END);
		if (ret == 0) {
			memset(sline, 0, sizeof(sline));
			sprintf(sline, "# Datum  --- Zeit      Temp[°C]  LDruck[hPa] Feuchte[rel]    Regen [mm]    Wind [m/s]");
			ret = write(stand->fdbme, sline, strlen(sline));
			if (ret == -1) {
				error_at_line(1,errno,__FILE__,__LINE__,"write()\n");
				return -errno;
			}
			ret = write(stand->fdbme, "\n", 1);
			if (ret == -1) {
				error_at_line(1,errno,__FILE__,__LINE__,"write()\n");
				return -errno;
			}
		}
	}

	ret = write(stand->fdbme, datetime, strlen(datetime));
	if (ret == -1) {
		error_at_line(1,errno,__FILE__,__LINE__,"write()\n");
		return -errno;
	}

	memset(sline, 0, sizeof(sline));
	sprintf(sline, "   %10.3f   %10.3f   %10.3f   %10.3f\n", 
				stand->temperature / 1000, stand->pressure * 10,
				stand->humidity, (stand->rain - stand->rain_alt));

	ret = write(stand->fdbme, sline, strlen(sline));
	if (ret == -1) {
		error_at_line(1,errno,__FILE__,__LINE__,"write()\n");
		return -errno;
	}

	snprintf(shm->bme.stemp, sizeof(shm->bme.stemp), "%d", lround(stand->temperature));
	shm->bme.temp = lround(stand->temperature);

	snprintf(shm->bme.spress, sizeof(shm->bme.spress), "%d", lround(stand->pressure * 10000));
	shm->bme.press = lround(stand->pressure);

	snprintf(shm->bme.shumi, sizeof(shm->bme.shumi), "%d", lround(stand->humidity));
	shm->bme.humi = lround(stand->humidity);

	snprintf(shm->bme.srain, sizeof(shm->bme.srain), "%d", lround(stand->rain*100));
	shm->bme.rain = lround(stand->rain);

	if (stand->rain >= 0.0)
		stand->rain_alt = stand->rain;

	for (i = 0; i < stand->nwaagen; i++) {

		memset(stand->waage[i]->ts, 0, sizeof(stand->waage[i]->ts));
		strncpy(stand->waage[i]->ts, ts, sizeof(stand->waage[i]->ts) - 1);

		ret = bw_set_waage_fileout(stand->waage[i]);
		if (ret == 0xAFFE) {
			// neues File aufgemacht
			ret = lseek(stand->waage[i]->fdout, 0, SEEK_END);
			if (ret == 0) {
				memset(sline, 0, sizeof(sline));
				sprintf(sline, "# Datum  --- Zeit     ");

				ret = write(stand->waage[i]->fdout, sline, strlen(sline));
				if (ret == -1)
					error_at_line(1,errno,__FILE__,__LINE__,"write()");
				memset(sline, 0, sizeof(sline));
				sprintf(sline, "Gewicht-%d [g]          ", i);
				ret = write(stand->waage[i]->fdout, sline, strlen(sline));
				if (ret == -1)
					error_at_line(1,errno,__FILE__,__LINE__,"write()");
				memset(sline, 0, sizeof(sline));
				sprintf(sline, "Tara-%d [g]          ", i);
				ret = write(stand->waage[i]->fdout, sline, strlen(sline));
				if (ret == -1)
					error_at_line(1,errno,__FILE__,__LINE__,"write()");
				ret = write(stand->waage[i]->fdout, "\n", 1);
				if (ret == -1)
					error_at_line(1,errno,__FILE__,__LINE__,"write()");
			}
		}

		ret = write(stand->waage[i]->fdout, datetime, strlen(datetime));
		if (ret == -1)
			error_at_line(1,errno,__FILE__,__LINE__,"write()");

		stand->waage[i]->gewicht = (stand->waage[i]->mass - stand->waage[i]->offset) *
				stand->waage[i]->scale - stand->waage[i]->tara; 
		memset(sline, 0, sizeof(sline));
		sprintf(sline, "  %10.0f  %8.0f  %8.0f %8.0f %12.6f\n", 
				stand->waage[i]->gewicht, 
				stand->waage[i]->tara,
				stand->waage[i]->mass,
				stand->waage[i]->offset,
				stand->waage[i]->scale);

		ret = write(stand->waage[i]->fdout, sline, strlen(sline));
		if (ret == -1)
			error_at_line(1,errno,__FILE__,__LINE__,"write()");

		snprintf(shm->waage[i].sgew, sizeof(shm->waage[i].sgew), "%d", lround(stand->waage[i]->gewicht));
		shm->waage[i].gewicht = lround(stand->waage[i]->gewicht);
	}

	return 0;
}

void db_disconnect(PGconn *conn, PGresult *res)
{
	fprintf(stderr, "%s\n", PQerrorMessage(conn));    
	PQfinish(conn);    
}


int db_connect(struct bw_stand *stand)
{
	stand->conn = PQconnectdb(stand->conn_string);
	if (PQstatus(stand->conn) == CONNECTION_BAD) {
		error_at_line(1,errno,__FILE__,__LINE__,
				"Connection to database failed: %s\n",
				PQerrorMessage(stand->conn));

		PQfinish(stand->conn);
		return -1;
	}

	return 0;
}

/*
 * ACHTUNG: write_bme_waagen_daten() muss davor aufgerufen werden, damit
 * gewicht und ts gefüllt sind
 */
int db_insert_waage(struct bw_stand *stand)
{
	PGresult *res;
	char *sql;
	int i;
	char ts[30];
	time_t time0;
	struct tm tm0;

	//###
	// nur zum Testen
	//
	   time(&time0);
	   localtime_r (&time0, &tm0);

	   memset(ts, 0, sizeof(ts));
	   sprintf(ts, "%04d-%02d-%02d %02d:%02d:%02d", 
	   tm0.tm_year + 1900, 
	   tm0.tm_mon + 1, tm0.tm_mday, 
	   tm0.tm_hour, tm0.tm_min,
	   tm0.tm_sec);
	   //
	//###

	for (i = 0; i < stand->nwaagen; i++) {
		//###
		//
		   memset(stand->waage[i]->ts, 0, sizeof(stand->waage[i]->ts));
		   strncpy(stand->waage[i]->ts, ts, sizeof(stand->waage[i]->ts) - 1);
		   //
		//###
		asprintf(&sql, "INSERT INTO gewicht (waa_id, value, ts) VALUES(%d, %9.0f, '%s'::timestamp)", 
				stand->waage[i]->waa_id, stand->waage[i]->gewicht, stand->waage[i]->ts);

		res = PQexec(stand->conn, sql);

		if (PQresultStatus(res) != PGRES_COMMAND_OK) {
			error_at_line(0,errno,__FILE__,__LINE__,
				"PQexec - NOK: %d\n", PQresultStatus(res));
		}

		free(sql);
		PQclear(res);    
	}

	return 0;
}

int main(int argn, char* argv[], char* envp[])
{
	int ret;
	int i;
	struct bw_stand *stand;
	struct pollfd pollfd[3];
	uint64_t exp;

	printf("Bienenwaage " __TIMESTAMP__ " gestartet\n");

	stand = malloc(sizeof(struct bw_stand));
	memset(stand, 0, sizeof(struct bw_stand));

	memset(stand->cfg_file, 0, sizeof(stand->cfg_file));
	strncpy(stand->cfg_file, bw_cfg_file, sizeof(stand->cfg_file) - 1);

	ret = init_stand(stand);
	if (ret < 0) {
		error_at_line(1,errno,__FILE__,__LINE__,"Fehler in init_stand(): cfg_file=%s\n", stand->cfg_file);
		return ret;
	}

	/* test of db_insert_waage() only */
	if ((argn >= 2) && (*argv[1] == 't')) {
		ret = db_connect(stand);
		if (ret < 0)
			error_at_line(1,errno,__FILE__,__LINE__,"db_connect() ret=%d\n", ret);

		ret = db_insert_waage(stand);
		if (ret < 0)
			error_at_line(1,errno,__FILE__,__LINE__,"db_insert_waage() ret=%d\n", ret);
	}

	ret = init_waagen_btn_tmr(stand);
	if (ret < 0) {
		error_at_line(1,errno,__FILE__,__LINE__,"Fehler in init_waagen()\n");
		return ret;
	}

	if (stand->db) {
		ret = db_connect(stand);
		if (ret < 0)
			error_at_line(1,errno,__FILE__,__LINE__,"db_connect() ret=%d\n", ret);
	}

	memset(pollfd, 0, sizeof(pollfd));
	pollfd[0].fd = stand->fdtmr;
	pollfd[0].events = POLLIN;

	while (1) {

		if (stand->zyklus) {
			ret = poll(pollfd, 1, -1);
			if (ret < 0)
				error_at_line(1,errno,__FILE__,__LINE__,"poll()");
			if (ret == 0)
				continue;

			ret = read(stand->fdtmr, &exp, sizeof(exp));
			if (ret == -1)
				error_at_line(1,errno,__FILE__,__LINE__,"read()\n");

			if (exp > 1) 
				error_at_line(0,errno,__FILE__,__LINE__,"Timer versäumt\n");
		}

		if (bw_ioctl_button(&stand->btn_pause) > 0)
			continue;

		ret = read_bme_daten(stand);
		if (ret < 0)
			error_at_line(1,errno,__FILE__,__LINE__,"read_bme_daten() ret=%d\n", ret);

		ret = read_waagen_daten(stand);
		if (ret < 0)
			error_at_line(1,errno,__FILE__,__LINE__,"read_waagen_daten() ret=%d\n", ret);

		ret = write_bme_waagen_daten(stand);
		if (ret < 0)
			error_at_line(1,errno,__FILE__,__LINE__,"write_waagen_daten() ret=%d\n", ret);

		if (stand->db) {
			ret = db_insert_waage(stand);
			if (ret < 0)
				error_at_line(1,errno,__FILE__,__LINE__,"db_insert_waage() ret=%d\n", ret);
		}
	}

	for (i = 0; i < stand->nwaagen; i++) {
		close(stand->waage[i]->fdout);
		free(stand->waage[i]);
	}

	close(stand->fdtmr);
	free(stand->waage);

	return 0;
}


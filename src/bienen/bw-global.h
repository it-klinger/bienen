/*
 * bw-global.h 
 *
 */

#ifndef __BW_GLOBAL_H
#define __BW_GLOBAL_H


#define error_exit(format, ...)							\
	({									\
		time_t time0 = time(NULL);					\
		fprintf(stderr, "%s", ctime(&time0));				\
		error_at_line(errno, errno, __FILE__, __LINE__, format,		\
								##__VA_ARGS__);	\
	 })

#define error_print(format, ...)						\
	({									\
		time_t time0 = time(NULL);					\
		fprintf(stderr, "%s", ctime(&time0));				\
		error_at_line(0, errno, __FILE__, __LINE__, format,		\
								##__VA_ARGS__);	\
		errno;								\
	 })

#define log_err(format, ...)							\
	({									\
		time_t time0 = time(NULL);					\
		fprintf(stderr, "%s %s", ctime(&time0), format, ##__VA_ARGS__);	\
	 })

#define BW_MAX_HX	10

#define BW_SHM_SIZE	4096

extern const char *bw_cfg_file;
extern const char *bw_iio_dir;
extern const char *bw_iio_device;

struct bw_btn {
	int		chip;
	int		nr;
	struct gpioevent_request
			req;
	int		fd;
	char		label[20];
};

struct bw_waage {
	int		idx;
	int		waa_id;
	char		title[20];
	char		dtb_name[20];
	char		dtb_node[20];
	char		iio_chan[20];
	double		offset;
	double		scale;
	double		tara;
	int		status;
	int		fd;
	double		mass;
	double		gewicht;
	char		ts[20];
	char		fnameout[50];
	int		fdout;
};

struct bw_stand {
	int		zyklus;
	int		thr_mass;
	int		fdtmr;
	int		nwaagen;
	int		fdshm;
	void*		shm;
	char		logfile[255];
	char		errfile[255];
	char		cfg_file[255];
	struct bw_btn	btn_pause;
	struct bw_waage	**waage;
	int		fdtemp;
	int		fdpres;
	int		fdhumi;
	int		fdrain;
	char		pressure_dtb_name[20];
	char		pressure_dtb_node[20];
	double		press_offset;
	char		fbmenameout[50];
	int		fdbme;
	double		temperature;
	double		humidity;
	double		pressure;
	double		rain;
	double		rain_alt;
	char		conn_string[255];
	PGconn		*conn;
	int		db;
};

struct bw_shm_header {
	char		ts[32];
	char		snwaagen[12];
	int		nwaagen;
};

struct bw_shm_bme {
	char		stemp[12];
	int		temp;
	char		spress[12];
	int		press;
	char		shumi[12];
	int		humi;
	char		srain[12];
	int		rain;
};

struct bw_shm_waage {
	char		sgew[12];
	int		gewicht;
};

struct bw_shm {
	struct bw_shm_header	head;
	struct bw_shm_bme	bme;
	struct bw_shm_waage	waage[BW_MAX_HX];
};

int bw_get_iio_device_idx(const char* name);

int bw_read_value(int fd, double *val);

int bw_read_value_rain(int fd, double *val);

int bw_init_button(struct bw_btn* button);

int bw_ioctl_button(struct bw_btn* button);

int bw_get_button(struct bw_btn* button);

#endif  /* __BW_GLOBAL_H */



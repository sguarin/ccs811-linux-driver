
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>


#define DEVICE_NAME "/dev/ccs811"
#define BASENAME(a) strrchr(a,'/') ? strrchr(a,'/') + 1 : a

#define SET_MODE 100
#define GET_MODE 101

/* globals */
char *program_name;
static int verbose_flag;

void usage()
{
	fprintf(stderr, "Usage: %s [options]\n", program_name);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\n\t-r, --read read \tread measurements\n");
	fprintf(stderr, "\n\t-m, --mode[=MODE] \tget or set measurement mode.\n");
	fprintf(stderr, "\t\t0 Idle (Measurements are disable in this mode\n");
	fprintf(stderr, "\t\t1 Constant power mode, IAQ measurement every second\n");
	fprintf(stderr, "\t\t2 Pulse heating mode, IAQ measurement every 10 seconds\n");
	fprintf(stderr, "\t\t3 Low power pulse heating mode, IAQ measurement every 60 seconds\n");
	fprintf(stderr, "\n\t-d, --device=DEVICE \tdevice to be used. (default /dev/ccs811)\n");
	fprintf(stderr, "\t-v, --verbose \tbe verbose where running\n");
	fprintf(stderr, "\t-h, --help print this help\n\n");
}

char *ccs811_read(char *device) {
	FILE *pfile;
	char *line;
	size_t len = 0;
	int ret;

	pfile = fopen(device, "r");
	if (pfile == NULL) {
		fprintf(stderr, "Error opening device %s\n", device);
		return NULL;
	}
	ret = getline(&line, &len, pfile);
	fclose(pfile);
	if (ret < 0) {
		printf("Read failed\n");
		return NULL;
	}
	printf("Read successfull\n");
	return line;
}

int set_mode(char *device, unsigned int mode) {
	int fd;
	int ret;
	fd = open(device, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Error opening device %s\n", device);
		return -1;
	}
	ret = ioctl(fd, SET_MODE, &mode);
	close(fd);
	if (ret < 0) {
		printf("Set mode failed %d\n", ret);
		return -1;
	}
	printf("Set mode successfull\n");
	return 0;
}

int get_mode(char *device) {
	int fd;
	int ret;
	unsigned int mode;
	fd = open(device, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Error opening device %s\n", device);
		return -1;
	}
	ret = ioctl(fd, GET_MODE, &mode);
	close(fd);
	if (ret < 0) {
		printf("Get mode failed %d\n", ret);
		return -1;
	}
	printf("Get mode successfull\n");
	return mode;
}

int
main (int argc, char **argv)
{
	int c;
	int val_mode;
	char *mode = NULL;
	char *data = NULL;

	int read_flag = 0;
	int mode_flag = 0;
	int help_flag = 0;

	char *device = DEVICE_NAME;

	program_name = BASENAME(argv[0]);

	while (1)
	{
		static struct option long_options[] =
		{
				/* These options set a flag. */
				{"verbose", no_argument,       &verbose_flag, 1},
				{"help",   no_argument,       0, 'h'},
				/* These options don’t set a flag.
             	 We distinguish them by their indices. */
				{"mode",    optional_argument,      0, 'm'},
				{"read",    no_argument,       		0, 'r'},
				{"device",  required_argument,      0, 'd'},
				{0, 0, 0, 0}
		};
		/* getopt_long stores the option index here. */
		int option_index = 0;

		c = getopt_long (argc, argv, "m::d:rvh",
				long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c)
		{
		case 'm':
			mode_flag = 1;
			if (optarg == NULL && argv[optind] != NULL
					&& argv[optind][0] != '-') {
				// not an option
				mode = strdup(argv[optind]);
				++optind;
			} else {
				// handle case of argument immediately after option
				if (optarg != NULL) {
					mode = strdup(optarg);
				}
			}

			break;

		case 'r':
			read_flag = 1;
			break;

		case 'd':
			device = strdup(optarg);
			break;

		case 'h':
			help_flag = 1;
			break;

		case '?':
			/* getopt_long already printed an error message. */
			break;

		default:
			abort ();
		}
	}


	if (mode_flag && read_flag) {
		printf("Incompatible options");
		usage();
		exit(1);
	}

	if (help_flag || (!mode_flag && !read_flag)) {
		usage();
		exit(1);
	}

	if (mode_flag) {
		if (mode != NULL) {
			/* set mode */
			val_mode = atol(mode);
			if (val_mode < 0 || val_mode > 3) {
				fprintf(stderr, "Error invalid mode\n");
			} else {
				set_mode(device, val_mode);
			}
		} else {
			/* Get mode */
			printf("Getting mode... ");
			val_mode = get_mode(device);
			printf("mode=%u\n", val_mode);
		}
	}

	if (read_flag) {
		printf("Reading... ");
		data = ccs811_read(device);
		if (data != NULL) {
			printf("values: %s", data);
		}
	}

	exit(0);
}


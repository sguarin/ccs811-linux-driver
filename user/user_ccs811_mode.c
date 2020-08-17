#include <stdlib.h>
#include <stdio.h>
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

char *program_name;

void usage()
{
        fprintf(stderr, "Usage: %s mode\n", program_name);
        fprintf(stderr, "\t0 Idle (Measurements are disable in this mode\n");
        fprintf(stderr, "\t1 Constant power mode, IAQ measurement every second\n");
        fprintf(stderr, "\t2 Pulse heating mode, IAQ measurement every 10 seconds\n");
        fprintf(stderr, "\t3 Low power pulse heating mode, IAQ measurement every 60 seconds\n");
        fprintf(stderr, "\n\n");
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
        int val_mode;
        char *mode;

        char *device = DEVICE_NAME;

        program_name = BASENAME(argv[0]);


        if (argc != 2 && argc != 1) {
                usage();
                exit(1);
        }

	if (argc == 2) {
        	mode = strdup(argv[1]);

                val_mode = atol(mode);
                if (val_mode < 0 || val_mode > 3) {
                        fprintf(stderr, "Error invalid mode\n");
                } else {
			printf(" Setting mode %d...", val_mode);
                        set_mode(device, val_mode);
                }
	} else if (argc == 1) {
		printf(" Getting mode ...");
		val_mode = get_mode(device);
		printf(" Mode=%u\n", val_mode);
	}
        exit(0);
}


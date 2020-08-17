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

#define BUFFER_LEN 32

char *program_name;

void usage()
{
        fprintf(stderr, "Usage: %s\n", program_name);
        fprintf(stderr, "\tReturns sensos values\n");
        fprintf(stderr, "\n\n");
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

int main (int argc, char **argv)
{
        char *data;

        char *device = DEVICE_NAME;

        program_name = BASENAME(argv[0]);


        if (argc != 1) {
                usage();
                exit(1);
        }

	printf("Reading...");
        data = ccs811_read(device);
	if (data != NULL) {
		printf("Values: %s", data);
	}
        exit(0);
}


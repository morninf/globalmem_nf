#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>



int main(int argc, char *argv)
{
	int fd;
	fd = open("/dev/globalmem",O_RDWR);
	if(fd<0)
	{
		printf("open device file fail\n");
		return 0;
	}
	printf("open file success\n");
	close(fd);
	return 0;
}

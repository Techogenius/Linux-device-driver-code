#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

// DEFINE THE ioctl CODE
#define WR_DATA _IOW('a', 'a', int32_t*)
//            magic number, command number, type
#define RD_DATA _IOR('a', 'b', int32_t*)

int main(void) {
	printf("\n----I O C T L----\nbased character device driver\noperation from user-space\n\n") ;
	int fd = open("/dev/my_device", O_RDWR) ;
	if(fd < 0) {
		printf("ERROR >< CANNOT OPEN THE DEVICE FILE\n") ;
		return 0 ;
	}

	int32_t num ;
	printf("ENTER THE DATA TO SEND : ") ;
	scanf("%d", &num) ;
	printf("WRITING VALUE TO THE DRIVER..\n") ;
	ioctl(fd, WR_DATA, (int32_t*)&num) ;

	int32_t val ;
	printf("\nREADING VALUE FROM THE DRIVER..\n") ;
	ioctl(fd, RD_DATA, (int32_t*)&val) ;
	printf("VALUE READ : %d\n", val) ;

	printf("\nCLOSING THE DRIVER\n\n") ;
	close(fd) ;

	return 0 ;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

// DEFINE THE ioctl CODE
#define WR_DATA _IOW('a', 'a', char*)
//            magic number, command number, type
#define RD_DATA _IOR('a', 'b', char*)

int main(void) {
	printf("\n----I O C T L----\nbased character device driver\noperation from user-space\n\n") ;
	int fd = open("/dev/my_device", O_RDWR) ;
	if(fd < 0) {
		printf("ERROR >< CANNOT OPEN THE DEVICE FILE\n") ;
		return 0 ;
	}

	char str[1024] ;
	printf("ENTER THE DATA TO SEND : ") ;
	scanf("%s", str) ;
	printf("WRITING VALUE TO THE DRIVER..\n") ;
	ioctl(fd, WR_DATA, str) ;

	char str_get[1024] ;
	printf("\nREADING VALUE FROM THE DRIVER..\n") ;
	ioctl(fd, RD_DATA, str_get) ;
	printf("VALUE READ : %s\n", str_get) ;

	printf("\nCLOSING THE DRIVER\n\n") ;
	close(fd) ;

	return 0 ;
}

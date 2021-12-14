#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(void){
	printf("WELCOME TO THE DEMO OF CHARACTER DEVICE DRIVER IN PROC FS\n\n") ;

	//int fd = open("/dev/chr_device", O_RDWR) ;
	//               ^we will not look at /dev. we will look at /proc

	int fd = open("/proc/chr_proc", O_RDWR) ;
	if(fd < 0){
		printf("ERROR >< CANNOT OPEN THE DEVICE FILE\n") ;
		return 1 ;
	}

	int option ;
	char write_buf[1024] ;
	char read_buf[1024] ;

	while(1){
		printf("----O P T I O N S----\n") ;
		printf("1 - WRITE\n") ;
		printf("2 - READ\n" ) ;
		printf("3 - EXIT\n\n" ) ;

		printf("YOUR CHOICE(1/2/3) : ") ;
		scanf("%d", &option) ;

		switch(option){
			case 1 :
				printf("\nENTER THE STRING TO WRITE INTO THE DRIVER : ") ;
				scanf(" %[^\t\n]s", write_buf) ;
				printf("DATA WRITTEN") ;
				write(fd, write_buf, strlen(write_buf)+1) ;
				printf("\nDONE\n\n") ;
				break ;
			case 2 :
				printf("\nREADING DATA\n") ;
				read(fd, read_buf, 1024) ;
				printf("DONE\n") ;
				printf("DATA READ :: %s\n\n", read_buf) ;
				break ;
			case 3 :
				close(fd) ;
				exit(1) ;
				break ;
			default :
				printf("WRONG CHOICE ENTERED !\n\n") ;
				break ;
		}
	}
	close(fd) ;
	return 0 ;
}

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/io.h>

#define BASEPORT 0x70

int main() {

char val =0;
/* Get access to the port */

if(ioperm(BASEPORT,4,1))
{
	perror("ioperm");
	exit(1);
}

/* Reading a byte */

val = inb(BASEPORT+1);
printf("seconds=%x\n",val);

/* We don't need the port anymore */


if(ioperm(BASEPORT,3,0))
{
	perror("ioperm");
	exit(1);
}

return 0;
}

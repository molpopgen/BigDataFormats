/*
  To compile:
  c++ -o binaryC binaryC.c -O2 -Wall -W

  Note: the -lm is implied by the C++ compiler, 
  but can be included. 
*/
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
/*
  Rather than stdio.h, we
  use the lower-level fcntl.h.
*/
#include <fcntl.h>
#include <unistd.h> /* Needed only on OS X */

int main( int argc, char ** argv )
{
  /* A buffer to store our stuff*/
  size_t MBUFFERSIZE = 1000000;
  double * dbuffer = (double *)malloc(MBUFFERSIZE*sizeof(double)),
   * dbuffer2 = (double *)malloc(MBUFFERSIZE*sizeof(double));
  FILE * fp;
  size_t i;
  /* file descriptor */
  int fd,rv;

  for( i = 0 ; i < MBUFFERSIZE ; ++i )
    {
      dbuffer[i] = sqrt(i)/((double)i); /*will force some inf*/
    }
  
  /*For convenience, use stdio.h routines to open file*/
  fp = fopen("testC.bin","wb");

  /*Get the "file descriptor" associated with the file handle*/
  fd = fileno(fp);

  /*Write the buffer to the file descriptor*/
  rv = write( fd, dbuffer, MBUFFERSIZE*sizeof(double) );

  printf("%d bytes written\n",rv);

  /*close the file*/
  fclose(fp);

  /*Now, read it back in...*/

  fp = fopen("testC.bin","rb");

  fd = fileno(fp);

  rv = read(fd, dbuffer2, MBUFFERSIZE*sizeof(double) );

  printf("%d bytes read\n",rv);

  for( i = 0 ; i < 10 ; ++i )
    {
      printf("Element %ld = %lf and %lf\n",i,dbuffer[i],dbuffer2[i]);
    }

  fclose(fp);

  /*
    Alternative approach to opening the file
    that only uses file descriptors

    For open(), PERMISSIONS MATTER!!!

    We are opening the file with user and group read/write permissions
    both set to "true".

    Failing to do so causes problems.
  */
  fd = open("testC_2.bin",O_RDWR|O_CREAT,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);

  if (fd == -1)
    {
      fprintf(stderr,"Error upon opening\n");
      exit(1);
    }

  write(fd,dbuffer,MBUFFERSIZE*sizeof(double));

  close(fd);
}

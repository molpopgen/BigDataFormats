"Big Data" formats/techniques for informatics programming
==============
This document covers various ways to handle large data sets, with an emphasis on "bionformatics" applications.  Rather than cover "standard" formats (BAM, VCF, etc.), the emphasis is on how a programmer may effectively handle large data sets using formats other than plain ASCII text.

Example code is centered on C/C++, and a basic understanding of those languages is assumed.

We will start with gzip output, which is probably what most people will think of when wishing to move beyond plain text.  gzip is a great format, but not necessarily the most convenient for programming in languages like C/C++.  As we move down the list of formats and techniques, we'll end up with formats that are more and more convenient, while also being very efficient.

#What features do we want in our "Big Data" programs?

1.  File output size isn't too big, but we want to write feweer larger files rather than millions of tiny files.
2.  We should generally write large blocks of data, rather than printing to a file every time we get an answer
3.  We should be able to seek within our files.  The ability to seek means that we can write a second (small) file telling us where every data record begins.  This "index" lets us rapidly move around the file to where records start, meaning we don't need to start reading from the top of the file each time we wish to find a specific data point.
4.  The format should not result in loss of precision.  Typically, when one writes floating-point numbers to a file, they are rounded.  We would like a format that avoids this, allowing us to read back in exatly what our program stored.

Gzipped
=======
A gzipped or ".gz" file is probably the simplest way to move away from plain text files.  The final output is the same, but the file size is much smaller.

You get .gz output via the use of the [zlib](http://zlib.net) run-time library.  This library is a C-language interface to gzip compression, and provideds function-for-function analogs to many of the C-library \<stdio.h\> read, write, fprintf, etc. that C programmers will be familiar with.

The zlib [manual](http://zlib.net/manual.html) is excellent and straightforward.  I recommend making sure that you have version >= 1.2.5 on your system, which provides the _gzbuffer_ function.

The basics (C):

```{c}
#include <zlib.h>
#include <stdlib.h>
#include <stdio.h>

int main( int argc, char ** argv )
{
  /*
    Open in write mode.  
    Use a for append mode.
    This is analzgous to fopen
  */
  gzFile  gzfp = gzopen("file.gz","wb"); 
  double x = 10.0;

  /*We need a buffer for input*/
  char buffer[sizeof(double)];
  int rv;

  /* gzprintf works just like fprintf */
  gzprintf( gzfp, "%lf\n", x );
  
  gzclose( gzfp ); /*same as fclose*/
  
  /*now, read it back in*/
  gzfp = gzopen("file.gz","rb");
  
  /* there is no gzscanf!!!, so we use gzread, and we must read into a char * buffer */
  rv=gzread( gzfp, buffer, sizeof(double) );
  x = -1.; /* set to -1 to prove that conversion works*/
  x = strtod(buffer,NULL);
  fprintf(stderr,"%d %lf %s\n",rv,x,buffer);
  fprintf(stderr,"%d %s\n",rv,buffer);
  
  exit(0);
}
```

As you can see, the absence of a _gzscanf_ function makes reading the data tiresome.

Of course, the basic use of zlib is identical in C++.  However, C++ make is very easy for us to buffer output from various sources.  The _std::ostringstream_ is more convenient than the C-language equavalent:

```{c++}
#include <zlib.h>

#include <sstream>
#include <iostream>

using namespace std;

int main( int argc, char ** argv )
{
  ostringstream buffer;

  gzFile out = gzopen("test.gz","wb");
  for( unsigned i = 0 ; i < 10 ; ++i )
    {
      buffer << "record number: " << i << '\n';
    }

  /*
    Write the buffer in one fell swoop.
    buffer.str().c_str() returns the char * is the buffer's internal data.
    buffer.str().size() is the size of the buffer in bytes b/c sizeof(char) = 1

    PRO tip:
    In more complex programs, write to buffer until buffer.str().size() >= some maximum value.
    At that point, write buffer to file, then reset buffer's internal data to empty string as follows:
    buffer.str( string() );

    The above command re-initializes the buffer with an empty string.
  */
  int rv = gzwrite( out, buffer.str().c_str(), buffer.str().size() );
  cerr << rv << " bytes written\n";

  //Be sure to close the file, else you may not see your data!!
  gzclose(out);
}
```

Pros of gzip:

1. Small output
2. Files are seekable and appendable.  This means you can build external indexes to rapidly find your data, etc.
3. Efficient

Cons:

1. Not totally convenient.  Have to read data back into buffers and then convert it to desired formats.  Bit of a drag.

##Note
There are many more advanced functions in zlib other than the higher-level analogs to the _stdio.h_ functions.  The library source comes with examples of use.  These functions are lower-level and more complex, but provider finer control over reading/writing from compressed streams.

##Compiling against zlib
Simply add this at link time:
```
-lz
```

For example:

```{sh}
cc -o gzexample gzexample.cc -O2 -Wall -W -lz
```

Binary
======
Rather than write data in plain text, we may prefer to write it in the native "binary" format that the computer natively understands.

To do so, we must convert our data to raw bits and write those bits to a file.  The result is a file that is not human-readable and contains no whitespace.

The pros of binary files are:

1.  Very fast to read/write from
2.  Smaller than plain text files
3.  Our data are not rounded
4.  No additional libraries are needed (support is built-into most languages).
5.  Files can be read by other languages, such as [R]{http://r-project.org}

The cons are:

1.  They are not human readable.
2.  Their format must be carefully documented in order to be read back in correctly.
3.  Strictly speaking, the files are not portable between systems (unless special care is taken regarding the sizes of data types used in programs).  However, the code to read/write them is portable.
4.  Because of points 1 and 2, errors in binary output can be tricky to track down.  Hint:  do plain-text output first, then modify the code to output binary later.
5.  You may want to write your data out differently from how you'd write a plain-text file.  More on this later.

The following two programs are identical in terms of what they are doing.  The first is in C and writes to files via file descriptors.  The second is in C++ and works via output streams.  Typically, I use a mix of C and C++ to try to maximize the convenience of object-oriented programming (C++) with the very fast I/O routines of C.

Example in C:

```{c}
/*
  To compile
  cc -o binaryC binaryC.c -O2 -Wall -W -lm
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
  size_t MBUFFERSIZE = 10000;
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
    that only uses file descriptors.
    
    Technially, all of our I/O can be
    done via fcntl/unistd, but that low-level
    world means dealing with file permissions 
    when opening, always knowing how many bytes
    you are writing, etc. (for example, to
    write a message to stderr).  Thus,
    there is a loss of convenience.

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
```

The example is trivially changed to C++ by replacing arrays with vectors, using the C++ versions of the headers, and declaring variables when we need them:

```{c++}
/*
  To compile:
  c++ -o binaryC binaryC.c -O2 -Wall -W

  Note: the -lm is implied by the C++ compiler, 
  but can be included. 
*/
#inc
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
  size_t MBUFFERSIZE = 10000;
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
```

The above is not very insightful.  Let's look at a C++ implementation that uses more of that language's features.  This example will introduce the following:

1. The use of reinterpret_cast to convert data types to a binary representation.
2. The use of the write() member function of streams for writing the binary representations.
3. Doing everything the "C++ way", _e.g._ doing everything with objects rather than C functions.

```{c++}
/*
  To compile:
  c++ -o binaryCpp2 binaryCpp2.c -O2 -Wall -W
  
  Note: the -lm is implied by the C++ compiler, 
  but can be included. 
 */
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <sstream> //used for buffering
#include <iostream> //for cout,cerr
#include <fstream> //use for C++ streams to files
/*
	Rather than stdio.h, we
	use the lower-level fcntl.h.
*/
#include <fcntl.h>
#include <unistd.h> /* Needed only on OS X */

using namespace std;

int main( int argc, char ** argv )
{
const size_t MBUFFER = 1024; //Max. buffer size in bytes
ostringstream buffer;

  	ofstream o("testCpp3.bin",ios::out | ios::binary );

  	for( size_t i = 0 ; i < 10000 ; ++i )
    	{
      		double x = sqrt(i)/double(i);
      		buffer.write( reinterpret_cast<char *>(&x), 
      				sizeof(double) );//write binary representation of x to buffer's stream
		if ( buffer.str().size() >= MBUFFER )
		//If buffer hits our max size in memory, print it to file and clear it.
		{
	  		cerr << "Writing buffer of length " << buffer.str().size() << '\n';
	  		o.write( buffer.str().c_str(), buffer.str().size() );
	  		buffer.str( string() );
		}
	}
  	//Pro tip: your amount of output % MBUFFER is probably != 0!!!
	if( !buffer.str().empty() )
    	{
      		o.write( buffer.str().c_str(), buffer.str().size() );
      		buffer.str( string() );
    	}
  	o.close();

  	//read it back in
  	vector<double> data;
  	ifstream in("testCpp3.bin",ios::in | ios::binary);
  	double x;
  	do
    	{
      		in.read( reinterpret_cast<char *>(&x), sizeof(double) );
      		if(!in.eof() && !in.fail())
		{
	  		data.push_back(x);
		}
    	}
  	while( ! in.eof() && !in.fail() );

  	cerr << data.size() << " doubles read from file\n";

  	for( size_t i = 0 ; i < 10 ; ++i )
    	{
      		cout << "Element " << i << " = " << sqrt(i)/double(i) << " and " << data[i] << '\n';
    	}
}
```

Gzipped binary
======


File locking
======



HDF5
=====

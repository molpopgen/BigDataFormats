"Big Data" formats/techniques for informatics programming
==============
This document covers various ways to handle large data sets, with an emphasis on "bionformatics" applications.  Rather than cover "standard" formats (BAM, VCF, etc.), the emphasis is on how a programmer may effectively handle large data sets using formats other than plain ASCII text.

Example code is centered on C/C++, and a basic understanding of those languages is assumed.

We will start with gzip output, which is probably what most people will think of when wishing to move beyond plain text.  gzip is a great format, but not necessarily the most convenient for programming in languages like C/C++.  As we move down the list of formats and techniques, we'll end up with formats that are more and more convenient, while also being very efficient.  However, not all formats are suitable for all purposes.

# Cliff's note version of how to choose a format

(Will come later)

# What features do we want in our "Big Data" programs?

1.  File output size isn't too big, but we want to write feweer larger files rather than millions of tiny files.
2.  We should generally write large blocks of data, rather than printing to a file every time we get an answer
3.  We should be able to seek within our files.  The ability to seek means that we can write a second (small) file telling us where every data record begins.  This "index" lets us rapidly move around the file to where records start, meaning we don't need to start reading from the top of the file each time we wish to find a specific data point.
4.  The format should not result in loss of precision.  Typically, when one writes floating-point numbers to a file, they are rounded.  We would like a format that avoids this, allowing us to read back in exatly what our program stored.

## How to seek, etc.

For very large data sets, some sort of meaningful index is handy.   The simplest index is a plain-text file that gives the position (in bytes) where a data record begins in a file.  For example:

|Record Name | Offset|
|----|:-----:|
| chr1 | 0 |
| chr2 | 1024 |
| chr3 | 4096 |

Typically, the offset will be stored as a long int.  This is fairly obvious in [C](http://www.cplusplus.com/reference/cstdio/ftell/).  It is a little more obsurce in C++, as the type is called [streampos](http://www.cplusplus.com/reference/ios/streampos/).  Usually, you can use long int or long long int, and most programs will be fine.

### C functions for seeking

There are two main functions as part of the standard language:

1. [ftell](http://www.cplusplus.com/reference/cstdio/ftell/) tells you where you are in a stream.
2. [fseek](http://www.cplusplus.com/reference/cstdio/fseek/) takes you from where you are to a different position in the stream.

The zlib library (see below) provides analagous functions:

(All function descriptions are direct copy/pastes from the zlib [manual])http://zlib.net/manual.html

1. gztell 
> Returns the starting position for the next gzread or gzwrite on the given compressed file. This position represents a number of bytes in the uncompressed data stream, and is zero when starting, even if appending or reading a gzip stream from the middle of a file using gzdopen().
2. gzoffset 
> Returns the current offset in the file being read or written. This offset includes the count of bytes that precede the gzip stream, for example when appending or when using gzdopen() for reading. When reading, the offset does not include as yet unused buffered input. This information can be used for a progress indicator. On error, gzoffset() returns –1.
3. gzseek 
> Sets the starting position for the next gzread or gzwrite on the given compressed file. The offset represents a number of bytes in the uncompressed data stream. The whence parameter is defined as in lseek(2); the value SEEK_END is not supported.

Some very important notes:

1. gzseek is emulated for files opened for reading.  It can be slow.  But, it exists, which is very useful.
2. gztell always returns 0 for a freshly-opened gzfile.  This happens _even if you open the file in append mode_!!!!

Point number two means that you cannot write an index file that looks like the above table.  However, you could write one that looks like this:

|Record Name | Size|
|----|:-----:|
| chr1 | 1024 |
| chr2 | 3072 |
| chr3 | 512 |

This index file actually contains the same data as the table in the previous section.  However, the second column no longer contains the offset where the i-th record begins.  Rather _it contains the size of the i-th record in bytes._  Therefore, the offset where the data for chr3 begins is the sum of all sizes preceeding the entry for chr3 in the index file.

### C++ functions for seeking

C++ lets you use the C functions described previously.  In addition to those functions, C++ streams have the following member functions:

1. [tellg](http://www.cplusplus.com/reference/istream/istream/tellg/) tells you where you are in an _input_ stream.
2. [tellp](http://www.cplusplus.com/reference/ostream/ostream/tellp/) tells you where you are in an _output_ stream.
3. [seekg](http://www.cplusplus.com/reference/istream/istream/seekg/) seeks to a position in an _input_ stream.
4. [seekp](http://www.cplusplus.com/reference/ostream/ostream/seekp/) seeks to a position in an _output_ stream.

### R functions for seeking
Whenever possible, I write my C/C++ programs so that the output can be read into R.  R has the ability to seek to positions in files:

```
#open for reading
f = file("infile.txt","r")
```

In the above block, file is a function returning what R calls a "connection".  See "help(connection)" for a list of the various types of connections that R supports.

Once open, you may seek within most types of connections:

```
seek( connection, offset)
```

In my experience, seeking in R works fine with the following types of connections:

1.  Plain-text files.
2.  Gzipped files.  These are connections open with gzfile("file.gz","r")
3.  Uncompressed binary.  file("file.bin","rb")
4.  Gzipped binary.  gzfile("file.bin.gz","rb")

For example, let's say you have lots of data frames in a plain-text file.  Prior to each data frame, there is an integer saying how many rows are in the following table.  The offsets from your index file refer to where this integer starts.  To read the i-th table:

```
f=file("file.txt","r")
seek(f, offset_to_ith_record)
nr=scan(f,nmax=1)
x = read.table(f,nrows=nr,header=TRUE)
```


## Making index files
In order to build an index file for your data, simply call the relevant tell functions before you write a new record.  Write these positions to an index file.  Then, use the appropriate seek function to move to the correct place.  There are trivial examples (C++) provided with this repo.  A real-world index file would be more than a list of offsets. Often, another column would be needed that identifies the record with some sort of unique ID.  (The need for an ID will be very important for situations where multiple processes write to the same file.  Often, record order in output files will become random.  See the file locking section below.)

## Examples of indexing

A (complex) example of a forward simulation writing uncompressed binary data to a file is [here](https://github.com/molpopgen/fwdpp/blob/master/examples/diploid_binaryIO_ind.cc).  This example includes the technique of POSIX file locking so that multiple instances of a simulation (for example, run on different nodes of a cluser using different random number seeds) can all write to a single output file.

Writing your plain-text data to a gzip output stream
=======
A gzipped or ".gz" file is probably the simplest way to move away from plain text files.  The final output is the same, but the file size is much smaller.  

You get .gz output via the use of the [zlib](http://zlib.net) run-time library.  This library is a C-language interface to gzip compression, and provideds function-for-function analogs to many of the C-library \<stdio.h\> read, write, fprintf, etc. that C programmers will be familiar with.

The zlib [manual](http://zlib.net/manual.html) is excellent and straightforward.  I recommend making sure that you have version >= 1.2.5 on your system, which provides the _gzbuffer_ function.

The most obvious application for a first-time zlib user will be to write your plain-text data to a gzipped file.   Doing so makes your files smaller, and they can still be read via standard command-line utilities like zless and zcat (or gunzip -C if you are on OS X).   This sort of output is the equivalent of writing your plain-text files as normal, then following up with a gzip command to compress them.  Doing so is fine, but does have side effects.  First, there is the extra time required.  Second, you need extra space for some period of time while gzip is doing its work.  Using zlib lets you skip the extra step.

This section concerns plain-text data being written to a compressed outputs stream using zlib.

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

1. Not totally convenient.  Have to read data back into buffers and then convert it to desired formats.  Bit of a drag.  (This con is specific to writing your plain-text data to a gzfile via gzprintf.  We'll fix this below).


## Compiling against zlib
Simply add this at link time:
```
-lz
```

For example:

```{sh}
cc -o gzexample gzexample.cc -O2 -Wall -W -lz
```

##Disclaimer/full disclosure

Technically, zlib supports _two_ types of data compression.  The first is the gzip format, which is accessed using zlib's functions with "gz" in the name.  This part of zlib provided the functions with analagous functionality to C's \<stdio.h\> header.  The second format supported by zlib is the zlib format.  The rest of the zlib library is devoted to that format.

The zlib and the gzip format are not the same!  I (KRT) have never used the zlib format and have zero idea if files written using that format are readable by R, etc.

## What about boost?

The [boost](http://www.boost.org) libraries have advanced stream classes that let you read/write gzip and bzip data.  These are super-awesome, in that they allow direct reading and writing to compressed files for any objects that have input and output operators defined for them.  However, the input streams are not seekable, which is a major drawback for us.  If you attempt to seek to a point in a gzip stream using boost, the stream goes into a bad/fail state.  I do not know why this is the case (zlib supports seeking, and is the base for the boost impementation), but it is what it is, so we move on.  Sad.

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
6.  A __big__ pro of binary format is how it treats floating point values.  When a calculation returns a non-finite value (inf or NaN, meaning infinity and not-a-number, respectively), writing those to a plain-text file does something interesting.  In a plain-text file, non-finite floating values are written in such a way that _they cannot be read back into floating-point data types directly_!  Rather, you must parse them as strings, check what the string says, and then use the correct [function](http://www.cplusplus.com/reference/cstdlib/strtold/) or [class](http://www.cplusplus.com/reference/limits/numeric_limits/) to return the desired floating-point equivalent.  Having to do this is obnoxious.  Worse, failure to do these conversions will often result in the input loop of your program hanging, becoming an infinite loop.  Binary files avoid this problem entirely.  An inf or a NaN are written to the file in their native representations, and can be read back out directly.  In other words, binary files help us meet some of our main goals of convenience and having a format that represents exactly what was stored by our program.

The cons are:

1.  They are not human readable.
2.  Their format must be carefully documented in order to be read back in correctly.
3.  Strictly speaking, the files are not portable between systems (unless special care is taken regarding the sizes of data types used in programs).  However, the code to read/write them is portable.
4.  Because of points 1 and 2, errors in binary output can be tricky to track down.  Hint:  do plain-text output first, then modify the code to output binary later.
5.  You may want to write your data out differently from how you'd write a plain-text file.  More on this later.
6.  They are nowhere near as small as a compressed format like zlib/gz.

## What are we going to learn?

In the examples, the following will become apparent:

1.  There are multiple ways to write binary output to files.  They boil down to C's "write" function and C++'s write function, which is a member function of stream classes.
2.  There are tradeoffs to make between convenience and efficiency.  More on this after the examples.

## Examples

The following two programs are identical in terms of what they are doing.  The first is in C and writes to files via file descriptors.  The second is in C++ and works via output streams.  Typically, I use a mix of C and C++ to try to maximize the convenience of object-oriented programming (C++) with the very fast I/O routines of C.

## Example in C:

The first example in C is [here](examples/binary/binaryC.c).

This example buffers data in a pointer to doubles and shows how to use either the low-level open function for creating files or how to use a mix of fopen/fileno to be able to later call write() for writing to files.

## Mixed C/C++ example
The example is trivially changed to C++ by replacing arrays with vectors, using the C++ versions of the headers, and declaring variables when we need them.

[Trivial C++ example](examples/binary/binaryCpp.cc)


## A "Full-C++" example
The above C++ example is not very insightful, as it basically uses the bare minimum of C++'s features.  Let's look at a C++ implementation that uses more of that language's features.  This example will introduce the following:

1. The use of reinterpret_cast to convert data types to a binary representation.
2. The use of the write() member function of streams for writing the binary representations.
3. Write to a buffer and flush the buffer to a file when it gets full.  This mimics what we want to do in real-world programs, which is to internally buffer large chunks of data in order to avoid small writes to files.
4. Doing everything the "C++ way", _e.g._ doing everything with objects rather than C functions.

#### ["Full-c++" example](examples/binary/binaryCpp2.cc)

This "full C++" example is easy to code, but which of the above is the fastest?  Testing on my powerbook (OS X Mavericks w/clang-503.0.40) gives the following benchmarks:

1. Full C version = 0.060 seconds
2. Trivially-C++ version = 0.067s
3. Full-C++ version = 0.252 seconds.

Why is the "full C++" the slowest?  A lot of it is due to the buffering using ostringstream.

##How you buffer matters!
We can get most of the speed back by buffering into a vector<double> rather than an ostringstream, which is shown in the following example.  (We get even more speed back by replacing C++ fstreams with C data types.)

This example takes 0.086 seconds on my machine, in between the fastest and slowest examples above:

#### [Example of buffering in a vector\<double\>](examples/binary/binaryCpp3.cc)

I take this as evidence that C++ input/output streams are not as slow as many people fear.

## Design considerations

Binary files are essentially a vomit of raw data to a file.  No white space, newlines, etc.  Worse, you cannot read them by eye, so you have to know the precise format of the output in order to read it back in.  That means you really need to document the format precisely.  Better yet, provide a function to read in the data using the same language used to write the data.

A big plus of binary data is that you can read directly into vectors.  For example, this "just works":

```c++
vector<double> x(NUMRECORDS); //assume NUMRECORDS is set, and is correct!
in.read( reinterpret_cast< char * >(&x[0]), NUMRECORDS * sizeof(double) );
```

Imagine you have data that you would store in a spreadsheet like this:

|Name | Index | Value|
|----|:-----:|-----|
| name1 | 0 | 0.001 |
| name2 | 7 | -1.23 |
| name3 | 5 | NaN |

It would be tedious to read in each string, then each integer, then each double, etc.  Instead, you may wish to have your output data organized like this:

1.  An unsigned integer stating how many rows there are.  Let's call this NROWS.
2.  Then, the NROWS index values.  These may be short, int, unsigned, etc., depending on your're needs.
3.  Then, the NROWS Value values.  Based on the table, these are some sort of floating-point value.
4.  Then, the NROWS Name value.

How does one write out all those names?  With no whitespace separator, you need to do one of two things:

1.  Write every string as a fixed-width number of characters.  I never do this.
2.  Write each string as an unsigned integer representing its length, followed by that many characters.
3.  Write all strings plus the null character '\0', and then read all string data until the '\0' is reached.

I always take the second option (although I should switch to the third...):

```c++
string x("I am a string!");
unsigned xlen = x.size();
out.write( reinterpret_cast<char *>(&xlen), sizeof(unsigned) );
out.write( x.c_str(), x.size() ); //no nead to multiply by sizeof(char) here...
```

That does make strings trickier to read back in:

```c++
vector<string> names(NROWS);
unsigned stringlen;
vector<char> temp;
for( unsigned i = 0 ; i < NROWS ; ++i )
	{
		in.read( reinterpret_cast<char *>(&stringlen), sizeof(unsigned) );
		if( stringlen > temp.size() )
		{
			//only allocate new space when necessary!
			temp.resize( stringlen );
		}
		in.read( reinterpret_cast< char * >(&temp[0]), stringlen*sizeof(char) );
		names[i].assign(temp.begin(),temp.begin() + stringlen);  //use iterator arithmetic in case stringlen < temp.size()!!!
	}
```

The above two code blocks can easily be done using the write and read functions in C, as shown in the above examples.

General guidelines for binary output are:

1.  Keep it simple.  Allow for entire vectors to be slurped in.
2.  If you want to read your files into [R](http://r-project.org), don't be too clever!  Any knowledgable programmer reading this has already realized that files can be made smaller by using types with fewer bits, etc.  That is true ([example](examples/binary/intSizes.cc)).  However, it means you really will need to document the format precisely, and you'll need to get intimate with how R treats variable sizes.  Personally, I stick to integers (signed and unsigned), floating-point, and character strings.  I skip short ints, bools, etc.
3.  If you desire max speed, buffer data into vectors of the specific type (see above).  Otherwise, buffer into an ostringstream because it is so easy.  (I do the latter usually--laziness FTW.)

## Examples of binary formats

Here are some examples from my own work.  Both of these use an approach similar to run-lenght encoding (RLE) to avoid writing out entire data blocks.

1.  [Reading/writing output from coalescent simulations](https://github.com/molpopgen/libsequence/blob/master/src/SimDataIO.cc).
2.  Output from forward simulation.  Specific simulation input/output is governed by custom policy classes.  Example is [here](https://github.com/molpopgen/fwdpp/blob/master/examples/diploid_binaryIO_ind.cc).

## Reading binary in R

For anything other than character data, use readBin:

```
#open for reading in binary mode
f = file("file.bin","rb")
#read in number of records
nrecs = readBin(f, "integer", 1)
#read in nrecs floating-points, which are sizeof(double)
x=readBin(f, "numeric", nrecs)
```

You can even read in a matrix:

```
f = file("file.bin","rb"); #open for reading in binary mode
ncol = readBin(f,"integer",1)
nrow = readBin(f,"integer",1)
m=matrix( readBin(f,"numeric", ncol*nrow), ncol = ncol, byrow = TRUE)
 ```

And yes, it is very fast.

To read in character data, use readChar (presumably after reading in the length of the string using readBin!):

```
name = readChar( f, 10 ) #reads in 10 characters
```

There are also writeBin and writeChar, for output.

Gzipped binary
======

The previous two sections covered zlib (in its most basic form) and introduced binary data as an output format.  Both have merits of their own can can be productively used for informatics application.  However, zlib's basic functions are a little tricky because when you gzprintf an integer or a float, there's no easy gzscanf to bring it back in.  Similarly, binary files are a problem because they actually aren't all that small.  Fortunately, it is trivial to merge the two types of output.   In fact, zlib natively supports this, by providing gzwrite(), an analog to the write() function used above!

Here is an example in C (the source is [here](examples/zlib/gzwrite.c)):

```{c}
/*
  Example of gzwrite using binary data

  cc -o gzwrite gzwrite.c -lm -lgz
 */
#include <zlib.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int main( int argc, char ** argv )
{
  size_t MAX = 100;
  double * x = (double *)malloc( MAX * sizeof( double ) ),
    *x2 = (double *)malloc( MAX * sizeof( double ) );

  gzFile gzfp = gzopen("out.gz","wb");
  size_t i;

  for( i = 0 ; i < MAX ; ++i )
    {
      x[i] = sin(i);
    }

  gzwrite( gzfp, x, MAX*sizeof(double) );
  gzclose( gzfp );

  gzfp = gzopen( "out.gz", "rb" );

  gzread(gzfp, x2, MAX*sizeof(double) );
  
  for( i = 0 ; i < MAX ; ++i )
    {
      fprintf(stdout,"%lf %lf\n",x[i],x2[i]);
    }
  free(x);
  free(x2);
  exit(0);
}
```

Executing the above program (compile it in the examples/zlib directory using "make", then run it) and then giving the following command:

```{sh}
zless out.gz
```

will print what looks like gibberish to the screen.  That gibberish is the sin function applied to the values 0 through 99, with the results written to the compressed stream in native binary format.

The extension to C++ should be obvious, as you just write your buffers using gzwrite.  Two methods may be used.

First:

```{c++}
vector< int > x;
//fill x somehow
gzwrite( gzstream, x.data(), x.size()*sizeof(int) );
```

Second:
```{c++}
ostringstream buffer;
//fill buffer by conversion to binary via reinterpret_cast
gzwrite( gzstream, buffer.str().c_str(), buffer.str().size() );
```

## Major caveat
This format is not ideal for all situations:

1.  See the notes above on the limitations of zlib's gztell function.  The only way to index a gz file is to read it after it has been created and record the starts of each record as you read through it.
2.  Because zlib hides the file descriptor from you, it is tricky to get this format to work nicely with POSIX file locking.

As a reult of these two points, our simulation programs that use binary output do not write compressed output.


HDF5
=====

On paper, the gzipped binary format described above will get you pretty far.  Either "gzbinary" or plain binary output are my go-to methods, and reflects the fact that a lot of the development that I do revolves around simulation.  For many computational biologists, the problems involve massive amounts of data that are a mix of text and numbers.  These include alignment data (SAM/BAM files in many cases), genotype data (VCF, of which there are at least two varieties), etc.

File locking 
======

## Small files = bad

This section describes a technique for managing output, not an output format. 
 
It is becoming more common for biology researchers to have access to large compute clusters.  These clusters are often campus resources shared between many campus units and therefore have to serve the needs of loads of users.  Some sort of queuing software (such as [OGS](http://gridscheduler.sourceforge.net/) or [SoGE](https://arc.liv.ac.uk/trac/SGE)) will match up a user's need with available resources and the job will run when resources become available. 
 
Large clusters often have some sort of network-mounted storage in the form of a distributed (or clustered) [file system](http://en.wikipedia.org/wiki/Clustered_file_system).  Such file systems make cluster use easier and can result in impressive I/O bandwidth.  However, there is one sure-fire way to overload such systems, and even bring them down.  A job that writes thousands of small files over a long period of time can wreak havok.  Performance of the file system will degrade cluster-wide, and the file system server may even crash.  (Note, different implementations of such file systems are differently-susceptible to this problem.  NFS and gluster don't handle "zillions of tiny files" well.  The [fhgfs](http://www.fhgfs.com/cms/)  system is much more robust in our experience. ) 
 
Here is an example of how to use a cluster to create gazillions of tiny files.  This is a hypothetical Grid Engine script that will run 100,000 replicates of a simulation.  Each replicate will be _written to a separate file_.  If this job gets access to hundreds of nodes, the file system may start to freak out and bad things may happen: 
 
 ```{sh} 
 #!/bin/bash 
 
 #$ -t 1-100000 
 
 cd $SGE_O_WORKDIR 
 SEED=`echo "$SGE_TASK_ID*$RANDOM"|bc -l` 
 simulate $SEED -o sim_output.$SGE_TASK_ID.txt 
 ```

How can one avoid writing all of these files, but still be able to use hundreds of cores to run the simulation via the simplicity of array jobs?  If you are programming for a [POSIX](http://en.wikipedia.org/wiki/POSIX) environment (Linux and OS X Mavericks are POSIX-compliant), then you may make use of low-level C funtions enabling "file locking".

## What is file locking?

File locking is a communication between your program and the OS kernel.  When a program wants to write, it may ask the kernel for an exclusive "lock" on the output file.  If no other process is currently locking the file, the requesting process gets the go-ahead and may write.  If the file is locked by another process, the requesting process may decide to wait or quit, etc.  After the process writes, it releases the lock so that another process may access it.

## How to do it (C/C++)?

You make use of the low-level file access function fcntl() in [\<fcntl.h\>](http://pubs.opengroup.org/onlinepubs/000095399/basedefs/fcntl.h.html).  This function lets you control all sorts of things regarding file access.

Some notes are needed:

1. fcntl() works via file descriptors rather than file pointers.  Thus, it is not compatible with C++ streams, which have no descriptor associated with them.  Learn how to use the C function fileno() to get the file descriptor associated with a FILE *!  (The statment about C++ refers to the language standard.  Various extensions exist that provide access to file descriptors.  Not portable, so we don't care.)

## Examples of file locking:

For my own work, the most common case use is simulation, where we need to generate a large number of replicates of the output of the same program.  Such tasks are amenable to array jobs and file locking helps prevent the "lots of output files" problem.

Examples:

1. [locking_routines](https://github.com/molpopgen/locking_routines) is a repo that I maintain so that I can re-use common locking operations.
2. [here](https://github.com/molpopgen/fwdpp/blob/master/examples/diploid_binaryIO_ind.cc) is a real-world example in the context of a simulation.

## Program design tips

1.  Do not request a lock until your program needs to write.
2.  Do not open files for writing until your program needs to write.  Buffer output (see above).  Request the lock when the buffer is full, and then open the file for writing.
3.  You shold open the files in append mode, else each open will truncate the file to 0 bytes!
4.  Close the file before releasing the lock.

Failure to do any of the above may result in "constipation", where loads of jobs are sitting around waiting for the lock to release.  See the examples for working code that I use routinely on clusters.

If you are building an index file along with your output file, the following ideas may come in useful:

1.  Simplify your life and make you index file either plain-text or uncompressed binary
2.  Request the lock on the index file only.  Then, write the data and update the index file, and release the lock on the index file.

## What about programs that you didn't write?

If a program prints to screen, and you cannot modify the code, try [atomic_locker](https://github.com/molpopgen/atomic_locker).  The UCI IT team have written a perl [program](http://moo.nac.uci.edu/~hjm/Job.Array.ZOT.html#_file_locking_with_multiple_processes_writing_to_a_single_file) with similar functionality.

If a program writes to an output file and does not use file locking, you may use named pipes and one of the above solutions:

```{sh}
mkfifo temp
program -o temp &
cat temp | atomic_locker id_number indexfilename outfilename
rm -f temp
```

The above command will buffer the output from "program" into a memory buffer called "temp".  We then cat that output through atomic\_locker and write it to a new file. 

In the context of an array job, each named pipe needs to be unique:

```{sh}
#!/bin/sh

#$ -q queuename
#$ -t 1-1000

cd $SGE_O_WORKDIR

#make the named pipe for job i:
PIPENAME=npipe.$SGE_TASK_ID
mkfifo $PIPENAME
#run the program, with $PIPENAME
program -o $PIPENAME &
#process the info buffered into the named pipe
cat $PIPENAME | atomic_locker $SGE_TASK_ID index.txt output.txt
#cleanup
rm -f $PIPENAME
```

Currently, atomic\_locker only supports plain text input/output.  Future versions will support gzip via zlib.

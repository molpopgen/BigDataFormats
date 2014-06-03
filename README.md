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
6.  A __big__ pro of binary format is how it treats floating point values.  When a calculation returns a non-finite value (inf or NaN, meaning infinity and not-a-number, respectively), writing those to a plain-text file does something interesting.  In a plain-text file, non-finite floating values are written in such a way that _they cannot be read back into floating-point data types directly_!  Rather, you must parse them as strings, check what the string says, and then use the correct [function](http://www.cplusplus.com/reference/cstdlib/strtold/) or [class](http://www.cplusplus.com/reference/limits/numeric_limits/) to return the desired floating-point equivalent.  Having to do this is obnoxious.  Worse, failure to do these conversions will often result in the input loop of your program hanging, becoming an infinite loop.  Binary files avoid this problem entirely.  An inf or a NaN are written to the file in their native representations, and can be read back out directly.  In other words, binary files help us meet some of our main goals of convenience and having a format that represents exactly what was stored by our program.

The cons are:

1.  They are not human readable.
2.  Their format must be carefully documented in order to be read back in correctly.
3.  Strictly speaking, the files are not portable between systems (unless special care is taken regarding the sizes of data types used in programs).  However, the code to read/write them is portable.
4.  Because of points 1 and 2, errors in binary output can be tricky to track down.  Hint:  do plain-text output first, then modify the code to output binary later.
5.  You may want to write your data out differently from how you'd write a plain-text file.  More on this later.
6.  They are nowhere near as small as a compressed format like zlib/gz.

##What are we going to learn?

In the examples, the following will become apparent:

1.  There are multiple ways to write binary output to files.  They boil down to C's "write" function and C++'s write function, which is a member function of stream classes.
2.  There are tradeoffs to make between convenience and efficiency.  More on this after the examples.

##Examples

The following two programs are identical in terms of what they are doing.  The first is in C and writes to files via file descriptors.  The second is in C++ and works via output streams.  Typically, I use a mix of C and C++ to try to maximize the convenience of object-oriented programming (C++) with the very fast I/O routines of C.

##Example in C:

The first example in C is [here](examples/binaryC.c).

This example buffers data in a pointer to doubles and shows how to use either the low-level open function for creating files or how to use a mix of fopen/fileno to be able to later call write() for writing to files.

##Mixed C/C++ example
The example is trivially changed to C++ by replacing arrays with vectors, using the C++ versions of the headers, and declaring variables when we need them.

[Trivial C++ example](examples/binaryCpp.cc)


##A "Full-C++" example
The above C++ example is not very insightful, as it basically uses the bare minimum of C++'s features.  Let's look at a C++ implementation that uses more of that language's features.  This example will introduce the following:

1. The use of reinterpret_cast to convert data types to a binary representation.
2. The use of the write() member function of streams for writing the binary representations.
3. Write to a buffer and flush the buffer to a file when it gets full.  This mimics what we want to do in real-world programs, which is to internally buffer large chunks of data in order to avoid small writes to files.
4. Doing everything the "C++ way", _e.g._ doing everything with objects rather than C functions.

["Full-c++" example](examples/binaryCpp2.cc)

This "full C++" example is easy to code, but which of the above is the fastest?  Testing on my powerbook (OS X Mavericks w/clang-503.0.40) gives the following benchmarks:

1. Full C version = 0.060 seconds
2. Trivially-C++ version = 0.067s
3. Full-C++ version = 0.252 seconds.

Why is the "full C++" the slowest?  A lot of it is due to the buffering using ostringstream.

##How you buffer matters!
We can get most of the speed back by buffering into a vector<double> rather than an ostringstream, which is shown in the following example.  (We get even more speed back by replacing C++ fstreams with C data types.)

This example takes 0.086 seconds on my machine, in between the fastest and slowest examples above:

[Example of buffering in a vector\<double\>](examples/binaryCpp3.cc)

I take this as evidence that C++ input/output streams are not as slow as many people fear.

##Design considerations

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

I always take the second option:

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
2.  If you want to read your files into [R](http://r-project.org), don't be too clever!  Any knowledgable programmer reading this has already realized that files can be made smaller by using types with fewer bits, etc.  That is true.  However, it means you really will need to document the format precisely, and you'll need to get intimate with how R treats variable sizes.  Personally, I stick to integers (signed and unsigned), floating-point, and character strings.  I skip short ints, bools, etc.

##Reading binary in R

For anything other than character data, use readBin:

```r 
 f = file("file.bin","rb"); #open for reading in binary mode
nrecs = readBin(f, "integer", 1) #read in number of records
x=readBin(f, "numeric", nrecs) #read in nrecs floating-points, which are sizeof(double)
```

You can even read in a matrix:

```r 
 f = file("file.bin","rb"); #open for reading in binary mode
 ncol = readBin(f,"integer",1)
 nrow = readBin(f,"integer",1)
 m=matrix( readBin(f,"numeric", ncol*nrow), ncol = ncol, byrow = TRUE)
 ```

And yes, it is very fast.

To read in character data, use readChar (presumably after reading in the length of the string using readBin!):

```r
name = readChar( f, 10 ) #reads in 10 characters
```


Gzipped binary
======


File locking
======



HDF5
=====

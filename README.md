"Big Data" formats/techniques for informatics programming
==============
This document covers various ways to handle large data sets, with an emphasis on "bionformatics" applications.  Rather than cover "standard" formats (BAM, VCF, etc.), the emphasis is on how a programmer may effectively handle large data sets using formats other than plain ASCII text.

Example code is centered on C/C++, and a basic understanding of those languages is assumed.

Gzipped
=======
A gzipped or ".gz" file is probably the simplest way to move away from plain text files.  The final output is the same, but the file size is much smaller.

You get .gz output via the use of the [zlib](http://zlib.net) run-time library.  This library is a C-language interface to gzip compression, and provideds function-for-function analogs to the C-library \<stdio.h\> read, write, fscanf, etc. that C programmers will be familiar with.

The zlib [manual](http://zlib.net/manual.html) is excellent and straightforward.  I recommend making sure that you have version >= 1.2.5 on your system, which provides the _gzbuffer_ function.

The basics (C):

```{c}
#include <zlib.h>
#include <stdlib.h>

int main( int argc, char ** argv )
{
  /*
    Open in write mode.  
    Use a for append mode.
    This is analzgous to fopen
  */
  gzFile * gzout = gzopen("file.gz","w"); 
  int x = 2
  /* gzprintf works just like fprintf */
  gzprintf( gzout, "%d\n", x );
  
  gzclose( gzout ); /*same as fclose*/
}

```


Binary
======


Gzipped binary
======


File locking
======



HDF5
=====

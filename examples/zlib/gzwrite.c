/*
  Example of gzwrite using binary data
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

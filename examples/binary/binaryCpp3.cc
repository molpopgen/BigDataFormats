/*
  To compile:
  c++ -o binaryCpp3 binaryCpp3.c -O2 -Wall -W
  
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
  const size_t MBUFFER = 1024; //Max. buffer size in 1024*sizeof(double) bytes
  vector<double> buffer;
  buffer.reserve( MBUFFER );
  ofstream o("testCpp4.bin",ios::out | ios::binary );

  for( size_t i = 0 ; i < 1000000 ; ++i )
    {
      buffer.push_back(sqrt(i)/double(i));
      if ( buffer.size() >= MBUFFER )
	//If buffer hits our max size in memory, print it to file and clear it.
	{
	  o.write( reinterpret_cast< char * >(&buffer[0]), buffer.size()*sizeof(double) );
	  buffer.clear();
	}
    }
  //Pro tip: your amount of output % MBUFFER is probably != 0!!!
  if( !buffer.empty() )
    {
      o.write( reinterpret_cast< char * >(&buffer[0]), buffer.size()*sizeof(double) );
      buffer.clear();
    }
  o.close();

  //read it back in
  vector<double> data;
  ifstream in("testCpp4.bin",ios::in | ios::binary);
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
      cout << "Element " << i << " = " << buffer[i] << " and " << data[i] << '\n';
    }
}

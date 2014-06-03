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
      buffer.write( reinterpret_cast<char *>(&x), sizeof(double) );//write binary representation of x to buffer's stream
      if ( buffer.str().size() >= MBUFFER )
	//If buffer hits our max size in memory, print it to file and clear it.
	{
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

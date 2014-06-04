/*
  Binary streams with integers of different sizes.

  The two output files will differ 4x in size.

  Requires C++11!
  
  c++ -std=c++11 -o intSizes intSizes.cc -O2 -Wall -W
 */
#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

//Let's use int8_t as an 8-bit int.
typedef int8_t smaller_int;

int main( int argc, char ** argv )
{
  vector< int > vi;    //our usual 32-bit int
  vector< smaller_int > vi8; //An 8-bit int

  //fill arrays
  for( int i = 0 ; i < INT8_MAX ; ++i )
    {
      vi.push_back(i);
    }

  for( int i = 0 ; i < INT8_MAX ; ++i )
    {
      //gotta convert.  Not possible (?) to use 
      //these small types in for loops
      vi8.push_back(smaller_int(i));
    }

  ofstream out1("intout.bin"),out2("int8out.bin");

  out1.write( reinterpret_cast<char *>( vi.data() ), vi.size() * sizeof(int) );
  out2.write( reinterpret_cast<char *>( vi8.data() ), vi8.size() * sizeof(smaller_int) );

  out1.close();
  out2.close();

  ifstream in1("intout.bin"),in2("int8out.bin");
  vector< int > vi_in(vi.size());    
  vector< smaller_int > vi8_in(vi8.size());

  in1.read( reinterpret_cast<char*>(vi_in.data()), vi_in.size()*sizeof(int) );
  in2.read( reinterpret_cast<char*>(vi8_in.data()), vi8_in.size()*sizeof(smaller_int) );

  for( size_t i = 0 ; i < vi.size() ; ++i )
    {
      cout << vi[i] << " -> " << vi_in[i] << ", and " << int(vi8[i]) << " -> " << int(vi8_in[i]) << '\n';
    }
}

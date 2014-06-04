//read in the 5th record from the output of make_index

#include <iostream>
#include <fstream>

using namespace std;

int main( int argc, char ** argv )
{
  ifstream index("index.txt"),input("output.txt");

  unsigned long offset;
  for( unsigned i = 0 ; i < 5 ; ++i )
    {
      index >> offset;
    }

  //seek to desired position in input stream
  input.seekg( offset );
  unsigned x;
  input >> x >> ws;
  cout << "The 5th record is " << x << endl;

  exit(0);
}

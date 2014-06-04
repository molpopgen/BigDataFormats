//write some data, and make an index for it.

#include <iostream>
#include <fstream>

using namespace std;

int main( int argc, char ** argv )
{
  ofstream output("output.txt"),outindex("index.txt");

  for( unsigned i = 0 ; i < 10 ; ++i )
    {
      outindex << output.tellp() << '\n'; //Write the offset of the i-th record
      output << i << '\n';  //write the data
    }
  outindex.close();
  output.close();
}

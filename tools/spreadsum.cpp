#include <hash/hash.hpp>

#include <fstream>
#include <vector>
#include <iostream>
#include <stdexcept>

using namespace std;
using namespace Spread;

Hash hashStream(istream &inf)
{
  vector<char> buf(1024);
  Hash res;

  while(!inf.eof())
    {
      // Read from input file
      inf.read(&buf[0], buf.size());
      if(inf.bad())
        return Hash();

      int count = inf.gcount();

      // Hash the read data
      res.update(&buf[0], count);
    }

  // We're done, return the hash
  return res.finish();
}

Hash hashFile(const std::string &src)
{
  // Input file
  ifstream inf(src.c_str(), ios::binary);
  if(!inf) return Hash();

  return hashStream(inf);
}

int main(int argc, char**argv)
{
  bool json = false;
  bool cleanup = false;

  /*
    0 = hex
    1 = base64
   */
  int format = 1;

  vector<string> files;

  // Search through parameters
  for(int i=1; i<argc; i++)
    {
      string par = argv[i];

      if(par == "-j")
        json = true;
      else if(par == "-c")
        cleanup = true;
      else if(par == "-b")
        format = 1;
      else if(par == "-x")
        format = 0;
      else if(par == "--help")
        {
          cout << "spreadsum [options] <files>\n\n";
          cout << "  -j    Output in JSON format\n";
          cout << "  -c    Remove leading ./ from filenames\n";
          cout << "  -x    Output in hex format\n";
          cout << "  -b    Output in base64 format (default)\n";
          return 0;
        }
      else
        files.push_back(par);
    }

  if(files.size() == 0)
    files.push_back("-");

  if(json)
    cout << "{\n";

  for(int i=0; i<files.size(); i++)
    {
      string file = files[i];

      if(cleanup && file.substr(0,2) == "./")
        file = file.substr(2);

      Hash h;
      if(file == "-")
        h = hashStream(cin);
      else
        h = hashFile(file);

      string hstring;

      if(format == 0)
        hstring = h.toHex();
      else if(format == 1)
        hstring = h.toString();

      if(json)
        {
          cout << "    \"" << file << "\":\"" << hstring << "\"";
          if(i+1 != files.size())
            cout << ",";
          cout << "\n";
        }
      else
        {
          // Pad compact values to look nice
          for(int i=hstring.size(); i<50; i++)
            hstring += " ";
          cout << hstring << "  " << file << endl;
        }
    }

  if(json)
    cout << "}\n";

  return 0;
}

#include <hash/hash.hpp>

#include <fstream>
#include <vector>
#include <iostream>
#include <stdexcept>

using namespace std;
using namespace Spread;

static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void b64_encodeblock( unsigned char in[3], char out[4], int len )
{
    out[0] = cb64[ in[0] >> 2 ];
    out[1] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
    out[2] = (len > 1 ? cb64[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ] : '=');
    out[3] = (len > 2 ? cb64[ in[2] & 0x3f ] : '=');
}

string base64(const void *input, int size)
{
  string res;

  // We know how large the output will be.
  res.reserve((4*size)/3 + 4);
  const unsigned char *inp = (unsigned char*)input;

  while(size)
    {
      unsigned char in[3];
      int len = 0;
      for(int i = 0; i < 3; i++ )
        {
          if(size)
            {
              in[i] = *(inp++);
              size--;
              len++;
            }
          else
            in[i] = 0;
        }

      assert(len);

      char out[4];
      b64_encodeblock( in, out, len );
      res.append(out, 4);
    }

  return res;
}

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
    2 = compact85
   */
  int format = 2;
  bool use64 = false;

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
      else if(par == "-a")
        format = 2;
      else if(par == "--help")
        {
          cout << "spreadsum [options] <files>\n\n";
          cout << "  -j    Output in JSON format\n";
          cout << "  -c    Remove leading ./ from filenames\n";
          cout << "  -x    Output in hex format\n";
          cout << "  -b    Output in base64 format\n";
          cout << "  -z    Output in compact85 (default)\n";
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
        hstring = base64(h.getData(), 40);
      else if(format == 2)
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

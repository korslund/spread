#include "readjson.hpp"
#include <iostream>

using namespace ReadJson;
using namespace std;

int main()
{
  Json::Value v = readJson("test.json");
  writeJson("test.json", v);
  cout << v << endl;
  cout << strToJson("[1,2,3]") << endl;
}

#include "readjson.hpp"

using namespace ReadJson;

int main()
{
  Json::Value v = readJson("test.json");
  writeJson("test.json", v);
}

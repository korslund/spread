#include "rule_loader.hpp"
#include "ruleset.hpp"
#include "misc/readjson.hpp"
#include <stdexcept>

using namespace Spread;

void fail(const std::string &str)
{
  throw std::runtime_error(str);
}

int Spread::loadRulesJson(RuleSet &rules, const Json::Value &val)
{
  if(!val.isArray()) return 0;
  int cnt = 0;
  for(int i=0; i<val.size(); i++)
    {
      if(val[i].isString())
        {
          addRule(rules, val[i].asString());
          cnt++;
        }
    }
  return cnt;
}

int Spread::loadRulesJsonFile(RuleSet &rules, const std::string &file)
{
  return loadRulesJson(rules, ReadJson::readJson(file));
}

static std::string getNext(const char* &ptr)
{
  while(*ptr == ' ') ptr++;
  const char *start = ptr;
  while(*ptr != ' ' && *ptr != 0) ptr++;
  return std::string(start, ptr-start);
}

static Hash getHash(const char* &ptr)
{
  std::string res = getNext(ptr);
  Hash h;
  if(res != "") h.fromString(res);
  return h;
}

bool isNum(const std::string &tmp)
{
  if(tmp == "") return false;

  char c = tmp[0];

  return (c >= '0' && c <= '9') || c == '.' || c == '-' || c == '+';
}

void Spread::decodeURL(const std::string &input, std::string &url,
                       int &prio, float &weight)
{
  prio = 1;
  weight = 1.0;

  const char *ptr = input.c_str();

  std::string tmp = getNext(ptr);
  if(isNum(tmp))
    {
      prio = atoi(tmp.c_str());

      tmp = getNext(ptr);
      if(isNum(tmp))
        {
          weight = atof(tmp.c_str());
          url = getNext(ptr);
        }
      else
        url = tmp;
    }
  else
    url = tmp;

  // 'url' now contains the first word in the URL. Add the rest, and
  // replace spaces with %20
  while(*ptr)
    {
      char c = *(ptr++);
      if(c == ' ')
        url += "%20";
      else
        url += c;
    }
}

void Spread::addRule(RuleSet &rules, const std::string &str)
{
  const char *ptr = str.c_str();

  // Decode rule string
  std::string tmp = getNext(ptr);

  if(tmp == "URL")
    {
      Hash hash = getHash(ptr);

      int prio;
      float w;
      std::string url;

      decodeURL(ptr, url, prio, w);

      if(hash.isNull() || url == "")
        fail("Invalid URL rule " + str);

       rules.addURL(hash, url, prio, w, str);
    }
  else if(tmp == "ARC")
    {
      Hash arcHash = getHash(ptr);
      Hash dirHash = getHash(ptr);

      if(arcHash.isNull() || dirHash.isNull())
        fail("Invalid ARC rule " + str);

      rules.addArchive(arcHash, dirHash, str);
    }
  else fail("Unknown rule " + str);
}

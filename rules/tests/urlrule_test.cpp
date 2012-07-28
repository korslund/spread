#include "common.cpp"

#include "ruleset.hpp"
#include <map>

Hash hello("hello",5);
Hash dolly("dolly",5);
RuleSet rules;

void test(const Hash &hash)
{
  test(hash, rules);
}

typedef std::map<std::string,int> Hist;

void stats(const Hash &hash)
{
  Hist hst;
  for(int i=0; i<10000; i++)
    {
      std::string url = URLRule::get(rules.findRule(hash))->url;
      hst[url]++;
    }

  Hist::iterator it;
  for(it = hst.begin(); it != hst.end(); it++)
    cout << it->first << "  :    " << it->second << endl;

  cout << endl;
}

struct Callback
{
  int i;

  void operator()(const Hash &hash, const std::string &url) const
  {
    cout << "  => Disabling " << h(hash) << " " << url
         << " (i=" << i << ")\n";
  }
};

void setCB()
{
  Callback cb;
  cb.i = 13;
  rules.setURLCallback(cb);
}

int main(int argc, char **argv)
{
  setCB();
  test(hello);
  cout << "Adding rule:\n";
  rules.addURL(hello, "http://hello1.com");
  test(hello);
  cout << "Disabling rule:\n";
  rules.reportBrokenURL(hello, "http://hello1.com");
  rules.reportBrokenURL(hello, "doesn't exist");
  test(hello);

  cout << "Adding rule twice:\n";
  rules.addURL(hello, "http://hello2.com", 2);
  rules.addURL(hello, "http://hello2.com", 3);
  test(hello);
  cout << "Adding something else:\n";
  rules.addURL(dolly, "DOLLY!");
  test(hello);
  test(dolly);
  cout << "Disabling both:\n";
  rules.reportBrokenURL(hello, "http://hello2.com");
  test(hello);

  cout << "Adding prio=5\n";
  rules.addURL(hello, "http://hello5.com", 5);
  test(hello);
  cout << "Adding prio=10\n";
  rules.addURL(hello, "http://hello10.com", 10);
  test(hello);
  cout << "Disabling prio=10:\n";
  rules.reportBrokenURL(hello, "http://hello10.com");
  test(hello);

  if(argc < 2)
    {
      cout << "\nAdd any parameter to include random tests.\n";
      return 0;
    }

  cout << "Adding weights at p=6\n";
  rules.addURL(hello, "random10_1", 6, 10);
  rules.addURL(hello, "random10_2", 6, 10);
  test(hello);
  stats(hello);

  cout << "Adding some more weights:\n";
  rules.addURL(hello, "random1   ", 6, 1);
  rules.addURL(hello, "random3   ", 6, 3);
  rules.addURL(hello, "random18  ", 6, 18);
  rules.addURL(hello, "random9   ", 6, 9);
  rules.addURL(hello, "random49  ", 6, 49);
  stats(hello);

  return 0;
}

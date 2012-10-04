#include "installer.hpp"
#include "build_actions.hpp"
#include "rules/urlrule.hpp"

using namespace Spread;

//#define DEBUG_PRINT
#ifdef DEBUG_PRINT
#include <iostream>
#define PRINT(a) std::cout << a << "\n";
#else
#define PRINT(a)
#endif

Installer::Installer(Cache::Cache &_cache, RuleSet &_rules,
                     const std::string &_prefix)
  : cache(_cache), rules(_rules)
{
  build.reset(new ActionBuilder(cache, rules, _prefix));
}

void Installer::addHint(const Hash &hint)
{ build->addHint(hint); }
void Installer::addDep(const std::string &file, const Hash &hash)
{ build->addDep(file,hash); }
void Installer::addDir(const Directory *dir)
{ build->addDir(dir); }
void Installer::addDir(const Hash &hash, bool alsoAsHint)
{ build->addDir(hash, alsoAsHint); }

void Installer::getActions(ActionMap &acts)
{ build->build(acts); }

void Installer::addToCache(const Hash &h, const std::string &file)
{
  PRINT("Caching " << file << "  hash=" << h);
  Hash res = cache.index.addFile(file, h);
  assert(res == h);
}

std::string Installer::getTmpFile(const Hash &h)
{
  std::string res = cache.createTmpFilename(h);
  PRINT("getTmpFile(" << h << ")  =>  " << res);
  return res;
}

std::string Installer::brokenURL(const Hash &hash, const std::string &url)
{
  PRINT("Reporting broken URL: " << url << "  hash=" << hash);
  rules.reportBrokenURL(hash, url);

  // Find a replacement
  const Rule *r = rules.findRule(hash);
  if(!r || r->type != RST_URL) return "";

  // Looks like we found one. Extract and return it.
  const std::string &newUrl = URLRule::get(r)->url;
  assert(newUrl != url);
  return newUrl;
}

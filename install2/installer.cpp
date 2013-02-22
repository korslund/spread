#include "installer.hpp"

using namespace Spread;

struct Installer::_Internal
{
  Internal(Cache::Cache &_cache, RuleSet &_rules, const std::string &_prefix)
  {}
};

Installer::Installer(Cache::Cache &cache, RuleSet &rules, const std::string &prefix)
{ ptr.reset(new _Internal(cache, rules, prefix)); }

void Installer::addFile(const std::string &file, const Hash &hash)
{
}

void Installer::remFile(const std::string &file, const Hash &hash)
{
}

void Installer::addDir(const Hash &hash, const std::string &path)
{
}

void Installer::remDir(const Hash &hash, const std::string &path)
{
}

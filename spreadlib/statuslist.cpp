#include "statuslist.hpp"
#include <misc/readjson.hpp>
#include <boost/filesystem.hpp>
#include "infojson.hpp"

using namespace Spread;
namespace bf = boost::filesystem;

typedef std::list<PackStatus> PSList;
typedef PSList::iterator PSLit;
typedef PSList::const_iterator PSLcit;

static void parent(const bf::path &file)
{
  bf::create_directories(file.parent_path());
}

struct StatusList::_Internal
{
  std::string confFile;

  PSList list;

  void parseJson(const Json::Value &root)
  {
    list.clear();

    if(!root.isArray()) return;

    for(int i=0; i<root.size(); i++)
      {
        const Json::Value &entry = root[i];

        PackStatus stat;
        jsonToInfo(stat.info, entry, entry["channel"].asString(),
                   entry["package"].asString());
        stat.where = entry["where"].asString();
        stat.needsUpdate = false;

        list.push_back(stat);
      }
  }

  Json::Value makeJson() const
  {
    Json::Value out;

    for(PSLcit it = list.begin(); it != list.end(); it++)
      {
        Json::Value entry;

        entry = infoToJson(it->info);
        entry["channel"] = it->info.channel;
        entry["package"] = it->info.package;
        entry["where"] = it->where;

        out.append(entry);
      }
    return out;
  }

  void loadConf(const std::string &file)
  {
    confFile = file;

    if(file == "") return;
    std::string toLoad = file;
    if(!bf::exists(toLoad))
      {
        toLoad += ".old";
        if(!bf::exists(toLoad))
          return;
      }

    parseJson(ReadJson::readJson(toLoad));
  }

  void save() const
  {
    if(confFile == "") return;

    Json::Value value = makeJson();

    // We follow the same save/load transactional logic as in
    // misc/jconfig, just haven't bothered to modularize it.
    std::string newFile = confFile + ".new";
    std::string oldFile = confFile + ".old";

    parent(newFile);
    ReadJson::writeJson(newFile, value);

    if(bf::exists(confFile))
    {
      if(bf::exists(oldFile))
        bf::remove(oldFile);
      bf::rename(confFile,oldFile);
    }
    bf::rename(newFile, confFile);
  }
};

StatusList::StatusList(const std::string &confFile)
{
  ptr.reset(new _Internal);
  try { ptr->loadConf(confFile); }
  catch(...) {}
}

static std::string killSlash(const std::string &path)
{
  int len = path.size();
  if(len > 1)
    {
      len--;
      char c = path[len];
      if(c == '/' || c == '\\')
        return path.substr(0,len);
    }
  return path;
}

static bool pMatch(const std::string &p1, const std::string &p2)
{
  return killSlash(p1) == killSlash(p2);
}

void StatusList::getList(PackStatusList &output,
                         const std::string &channel,
                         const std::string &package,
                         const std::string &where) const
{
  for(PSLcit it = ptr->list.begin(); it != ptr->list.end(); it++)
    {
      // Skip non-matching entries
      if(channel != "" && channel != it->info.channel) continue;
      if(package != "" && package != it->info.package) continue;
      if(where != "" && !pMatch(where,it->where)) continue;

      output.push_back(&*it);
    }
}

const PackStatus *StatusList::get(const std::string &channel,
                                  const std::string &package,
                                  const std::string &where) const
{
  for(PSLcit it = ptr->list.begin(); it != ptr->list.end(); it++)
    {
      if(it->info.channel == channel && it->info.package == package &&
         (where == "" || pMatch(it->where,where)))
        return &*it;
    }
  return NULL;
}

void StatusList::setEntry(const PackInfo &info, const std::string &where)
{
  assert(where != "");

  // Loop through and look for matches
  for(PSLit it = ptr->list.begin(); it != ptr->list.end(); it++)
    {
      if(it->info.channel == info.channel && it->info.package == info.package &&
         pMatch(it->where,where))
        {
          it->info = info;
          it->where = where;
          it->needsUpdate = false;
          ptr->save();
          return;
        }
    }

  // No match was found, add a new entry
  PackStatus stat;
  stat.info = info;
  stat.where = where;
  stat.needsUpdate = false;
  ptr->list.push_back(stat);
  ptr->save();
  return;
}

void StatusList::remove(const std::string &channel, const std::string &package,
                        const std::string &where)
{
  // Loop and remove matches
  for(PSLit it = ptr->list.begin(); it != ptr->list.end(); it++)
    {
      if(it->info.channel == channel && it->info.package == package)
        {
          // Match location, or the first pack found if no location
          // was given
          if(where == "" || pMatch(it->where, where))
            {
              ptr->list.erase(it);
              ptr->save();
              return;
            }
        }
    }
}

void StatusList::notifyNew(const PackInfo &info)
{
  assert(info.dirs.size() == info.paths.size());
  assert(info.channel != "" && info.package != "");
  for(PSLit it = ptr->list.begin(); it != ptr->list.end(); it++)
    {
      assert(it->info.dirs.size() == it->info.paths.size());
      if(it->info.channel == info.channel && it->info.package == info.package)
        {
          // Assume inequality, unless we pass the equality tests
          // below
          it->needsUpdate = true;

          if(it->info.dirs.size() != info.dirs.size())
            continue;

          bool fail = false;
          for(int i=0; i<info.dirs.size(); i++)
            {
              if(info.dirs[i] != it->info.dirs[i] ||
                 info.paths[i] != it->info.paths[i])
                {
                  fail = true;
                  break;
                }
            }
          if(fail) continue;

          // The installed pack matches the most recent pack info,
          // therefore the installed back does NOT need to be updated.
          it->needsUpdate = false;
        }
    }
}

/* Since we are using direct search (no lookups) in notifyNew, this
   function in particular isn't very efficient for large list
   sizes. We assume for now that the number of installed games is
   relatively low. We can optimize this using map<>s for lookups
   later.
 */
void StatusList::notifyNew(const PackInfoList &list)
{
  for(int i=0; i<list.size(); i++)
    notifyNew(list[i]);
}

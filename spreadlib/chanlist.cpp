#include "chanlist.hpp"
#include <stdexcept>
#include <map>
#include <boost/filesystem.hpp>
#include <rules/rule_loader.hpp>

using namespace Spread;
namespace bf = boost::filesystem;

static void fail(const std::string &msg)
{ throw std::runtime_error(msg); }

typedef std::map<std::string, PackList> PackMap;
typedef std::map<std::string, JobInfoPtr> JobMap;

struct ChanList::_Internal
{
  PackMap packs;
  JobMap chanJobs;
  StatusList status;

  RuleSet &rules;

  bf::path basedir;

  _Internal(const std::string &statFile, RuleSet &r)
    : status(statFile), rules(r) {}

  PackList *getP(const std::string &channel)
  {
    PackMap::iterator it = packs.find(channel);
    if(it == packs.end()) return NULL;
    return &it->second;
  }

  std::string chanPath(const std::string &channel, const std::string &path)
  {
    if(channel.find('/') != std::string::npos ||
       channel.find('\\') != std::string::npos)
      fail("Invalid channel name " + channel + " (contains slashes)");

    return (basedir / "channels" / channel / path).string();
  }

  /* Loop through chanJobs and call update() for all finished
     jobs. This is called before fetching the status list, to make
     sure the various 'needsUpdate' flags are updated with the latest
     known information.
   */
  void updateAll()
  {
    JobMap::const_iterator it;
    for(it = chanJobs.begin(); it != chanJobs.end(); it++)
      if(it->second && it->second->isSuccess())
        update(it->first);
  }

  // Load the newest ruleset and packlist, if it has been updated
  // since our last run or if it hasn't been loaded at all yet.
  void update(const std::string &channel)
  {
    PackList *plist = getP(channel);

    // Does the channel exist?
    if(plist)
      {
        // Yes. Check if it is currently being updated.
        JobInfoPtr &info = chanJobs[channel];

        // If there has been no updates since our last load, there's
        // no point in reloading.
        if(!info) return;

        if(info->isSuccess())
          {
            // The job is done, so the data on disk is usable.
            info.reset();
            assert(!chanJobs[channel]);
          }
        else
          {
            /* The job is currently writing the data (or it has
               failed), meaning we can't safely load it right now.
            */
            return;
          }
      }

    try
      {
        loadRulesJsonFile(rules, chanPath(channel, "rules.json"));
        PackList list;
        list.loadJson(chanPath(channel, "packs.json"), channel);

        // Assign in a separate step after loading, otherwise an
        // exception above puts us in an unpredictable state. PackList
        // is very lightweight so this is OK.
        packs[channel] = list;

        // Notify the status list of all our new package data
        plist = getP(channel);
        assert(plist);
        status.notifyNew(plist->getList());
      }
    catch(std::exception &e)
      {
        fail("Error loading channel " + channel + ".\nDETAILS: " + e.what());
      }
  }
};

ChanList::ChanList(const std::string &basedir, RuleSet &r)
{
  bf::path dir(basedir);
  dir /= "installed.conf";
  ptr.reset(new _Internal(dir.string(), r));
  ptr->basedir = basedir;
}

void ChanList::load(const std::string &channel)
{
  ptr->update(channel);
}

const PackList &ChanList::getPackList(const std::string &channel)
{
  ptr->update(channel);
  PackList *p = ptr->getP(channel);
  if(!p) fail("Channel not found: " + channel);
  return *p;
}

StatusList &ChanList::getStatusList()
{
  ptr->updateAll();
  return ptr->status;
}

void ChanList::setChannelJob(const std::string &channel, JobInfoPtr job)
{
  JobInfoPtr cur = ptr->chanJobs[channel];
  if(cur && !cur->isFinished())
    fail("Update already in progress for channel '" + channel + "'");
  ptr->chanJobs[channel] = job;
}

#include "ruleset.hpp"
#include "urlrule.hpp"
#include <vector>
#include <map>
#include "misc/random.hpp"
#include <boost/thread/recursive_mutex.hpp>
#include <cstdio>

using namespace Spread;

typedef boost::shared_ptr<URLRule> URLPtr;
typedef boost::shared_ptr<ArcRuleData> ArcPtr;
typedef std::vector<URLPtr> UVec;
typedef std::map<Hash, UVec> UMap;
typedef std::map<Hash, ArcPtr> AMap;

#define LOCK boost::lock_guard<boost::recursive_mutex> lock(ptr->mutex)

struct RuleSet::_RuleSetInternal
{
  UMap urls;
  AMap arcs;

  Misc::Random rnd;
  boost::recursive_mutex mutex;

  const ArcRuleData* findArc(const Hash &hash) const
  {
    AMap::const_iterator it = arcs.find(hash);
    if(it == arcs.end())
      return NULL;

    return it->second.get();
  }

  // Adds all URL rules for 'hash' to the given vector
  void addURLs(const Hash &hash, RuleList &output) const
  {
    UMap::const_iterator it = urls.find(hash);
    if(it == urls.end()) return;

    const UVec &vec = it->second;
    if(!vec.size()) return;

    for(int i=0; i<vec.size(); i++)
      {
        const URLRule *r = vec[i].get();

        // Skip disabled rules
        if(r->isBroken) continue;

        // Add it!
        output.insert(r);
      }
  }

  // Returns an URL rule or NULL
  const Rule* findURL(const Hash &hash)
  {
    UMap::const_iterator it = urls.find(hash);
    if(it == urls.end())
      return NULL;

    const UVec &vec = it->second;

    if(!vec.size())
      return NULL;

    // First, build a list of rules with the highest priority
    int prio;
    bool prioFirst = true;
    std::vector<int> indices;
    double sum = 0;

    for(int i=0; i<vec.size(); i++)
      {
        const URLRule &r = *vec[i].get();

        // Skip disabled rules
        if(r.isBroken) continue;

        if(prioFirst)
          {
            prio = r.priority;
            prioFirst = false;
          }

        // Drop all previous results if this has a higher priority
        else if(r.priority > prio)
          {
            indices.clear();
            prio = r.priority;
            sum = 0;
          }

        // Skip everything below the highest priority level
        else if(r.priority < prio)
          continue;

        // Add this to the weight list
        if(r.weight > 0) sum += r.weight;
        indices.push_back(i);
      }

    if(indices.size() == 0)
      return NULL;

    /* Pick a card, any card!

       Generates a randum number between 0 and 'sum'.
     */
    double pick = rnd.dgen()*sum;

    // Pick the right one
    double psum = 0;
    for(int i=0; i<indices.size(); i++)
      {
        URLPtr ptr = vec[indices[i]];
        float w = ptr->weight;
        if(w <= 0) continue;
        psum += w;

        if(pick < psum)
          return ptr.get();
      }

    /* In case our impeccable math skillz failed, just go for the
       first one. This is also simpler than trying to account for
       pathological corner cases such as NaN weights etc.
    */
    return vec[indices[0]].get();
  }
};

RuleSet::RuleSet()
{
  ptr.reset(new _RuleSetInternal);
}

void RuleSet::findAllRules(const Hash &hash, RuleList &output) const
{
  LOCK;
  ptr->addURLs(hash, output);
}

const std::vector<Hash>* RuleSet::findHints(const Hash &dirHash) const
{
  LOCK;
  // Currently not implemented
  return NULL;
}

const Rule *RuleSet::findRule(const Hash &hash) const
{
  LOCK;

  // Search URLs only
  const Rule *rule = ptr->findURL(hash);
  if(rule) return rule;

  return NULL;
}

const ArcRuleData *RuleSet::findArchive(const Hash &hash) const
{
  LOCK;
  return ptr->findArc(hash);
}

void RuleSet::addArchive(const Hash &hash, const Hash &dirHash,
                         std::string ruleString)
{
  if(ruleString == "")
    ruleString = "ARC " + hash.toString() + " " + dirHash.toString();

  LOCK;
  ArcPtr data(new ArcRuleData(hash, dirHash, ruleString));

  // Make the archive retrievable both through the archive hash and
  // the dir hash.
  ptr->arcs[hash] = data;
  ptr->arcs[dirHash] = data;
}

void RuleSet::addURL(const Hash &hash, const std::string &url,
                     int priority, float weight,
                     std::string ruleString)
{
  if(ruleString == "")
    {
      ruleString = "URL " + hash.toString() + " ";

      bool hasWeight = (weight != 1.0);

      char buf[40];
      if(priority != 1 || hasWeight)
        {
          std::snprintf(buf,40,"%d ", priority);
          ruleString += buf;
        }
      if(hasWeight)
        {
          std::snprintf(buf,40,"%f ", weight);
          ruleString += buf;
        }

      ruleString += url;
    }

  LOCK;
  ptr->urls[hash].push_back
    (URLPtr(new URLRule(hash, ruleString, url, priority, weight)));
}

void RuleSet::reportBrokenURL(const Hash &hash, const std::string &url)
{
  LOCK;

  // Find all matching rules and disable them
  UMap::iterator it = ptr->urls.find(hash);
  if(it != ptr->urls.end())
    {
      UVec &vec = it->second;

      for(int i=0; i<vec.size(); i++)
        {
          URLRule *r = vec[i].get();
          if(r->url == url)
            r->isBroken = true;
        }
    }

  // Invoke the callback to notify external systems about the broken
  // URL.
  if(callback) callback(hash, url);
}

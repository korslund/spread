#include "sr0.hpp"
#include "tasks/install.hpp"
#include "tasks/fileop.hpp"
#include <boost/filesystem.hpp>
#include <mangle/stream/servers/string_writer.hpp>
#include "unpack/auto.hpp"
#include "misc/readjson.hpp"
#include "hash/hash_stream.hpp"
#include <map>
#include <set>

using namespace Tasks;
using namespace SR0;
namespace bs = boost::filesystem;
using namespace Mangle::Stream;
using namespace Mangle::VFS;
using namespace Spread;

typedef std::map<std::string,std::string> StrMap;

/* Converts an archive file into a map of strings. Use the static
   GetArc().
 */
struct Stringify : StreamFactory
{
  StrMap &files;

  Stringify(StrMap &smap) : files(smap) {}

  StreamPtr open(const std::string &name)
  {
    std::string &str = files[name];
    Stream *out = new StringWriter(str);
    return StreamPtr(out);
  }

  static void GetArc(const std::string &arcfile,
                     StrMap &outlist)
  {
    Unpack::AutoUnpack unp;
    unp.unpack(arcfile, StreamFactoryPtr(new Stringify(outlist)));
  }
};

void fail(const std::string &msg)
{
  throw std::runtime_error("SR0 Error: " + msg);
}

#include <iostream>

typedef std::map<Hash, std::string> HashDir;

struct RuleParser;
struct Rule
{
  virtual void index(RuleParser*) = 0;

  /* This is not a usable solution, we need something more akin to the
     job tree we've prototyped elsewhere.

     I can imagine that we have a generalized task collector that you
     provide with input files (and their known hashes), wanted output
     files and their hashes, and then you run that. The output may
     contain duplicate hashes, meaning that the same content should be
     copied into more than one place.

     There is one type specialized for each rule. The tree task system
     is rule- independent, and uses this for the actual tasks. The
     tree system is only responsible for getting necessary
     dependencies, setting up temporary files (if necessary), and
     stringing the tasks together in the right order.
  */
  virtual void writeTo(const std::string &where) = 0;
};

struct RuleParser
{
  typedef std::set<Rule*> RuleSet;
  typedef std::map<Hash, Rule*> RuleMap;

  RuleSet rules;
  RuleMap targets;

  RuleParser() {}

  RuleParser(const Json::Value &val)
  {
    for(int i=0; i<val.size(); i++)
      add(val[i].asString());
  }

  void add(const std::string &rulestr);

  void addTarget(Hash h, Rule *r)
  {
    // TODO: In this very simple test, there's only one rule per
    // target.
    targets[h] = r;
  }
};

struct TestRule : Rule
{
  Hash hash;
  std::string file;

  TestRule(RuleParser *owner, const std::string &str)
  {
    using namespace std;

    int split = str.find(' ');
    if(split == string::npos)
      fail("Invalid TEST rule string '" + str + "'");

    hash.fromString(str.substr(0, split));
    file = str.substr(split+1);

    cout << "Created rule TEST " << hash << " => " << file << endl;
  }

  void index(RuleParser *list)
  {
    list->addTarget(hash, this);
  }

  void writeTo(const std::string &where)
  {
    Tasks::FileOpTask op;
    op.copy(file,where);
    op.run();
    if(op.getInfo()->isNonSuccess())
      fail("Failed to create " + where);
  }
};

void RuleParser::add(const std::string &rulestr)
{
  // The rule 'verb' is the word before the first space
  int pos = rulestr.find(' ');
  std::string verb = rulestr.substr(0, pos);
  std::string rest;
  if(rulestr.size() > verb.size())
    rest = rulestr.substr(pos+1);

  Rule* rule = NULL;
  if(verb == "TEST")
    rule = new TestRule(this, rest);

  rules.insert(rule);
  rule->index(this);
}

static void applySingle(const Json::Value &val, const std::string &file, bs::path dir,
                        RuleParser &rules)
{
  using namespace std;

  if(!val.isMember(file))
    return;

  // Get new file hash
  Hash newHash(val[file].asString());

  // Hash existing file, if any
  Hash oldHash;

  std::string realfile = (dir/file).string();
  if(bs::exists(realfile))
    HashStream::sum(realfile);

  // If it's already up-to-date, exit
  if(oldHash == newHash) return;

  cout << "Updating " << realfile << " from "
       << oldHash << " to " << newHash << endl;

  Rule *r = rules.targets[newHash];
  assert(r);
  r->writeTo(realfile);
}

void SR0::applyFile(const std::string &dir, const std::string &file)
{
  StrMap files;
  Stringify::GetArc(file, files);
  std::string contents = files["index.json"];

  if(contents == "")
    fail("Missing index.json");

  Json::Value val = ReadJson::strToJson(contents);

  RuleParser rules(val["rules"]);

  applySingle(val, "names.json", dir, rules);
  applySingle(val, "rules.json", dir, rules);
}

void SR0::applyURL(const std::string &dir, const std::string &url)
{
  std::string file = (bs::path(dir)/"_sr0_tmp.zip").string();

  InstallTask getFile(url, file, "");
  getFile.run();
  if(!getFile.getInfo()->isSuccess())
    fail("Failed downloading " + url);

  applyFile(dir, file);
  bs::remove(file);
}

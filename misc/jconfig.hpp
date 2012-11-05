#ifndef __JCONFIG_HPP_
#define __JCONFIG_HPP_

#include <string>
#include <vector>
#include <map>
#include <stdint.h>

namespace Misc
{
  /* This struct reads and writes config to a json file.
   */
  struct JConfig
  {
    bool readOnly;

    JConfig(const std::string &_file = "", bool _readOnly=false);
    ~JConfig();

    void load();
    void save();

    void load(const std::string &_file)
    { file = _file; load(); }
    void save(const std::string &_file)
    { file = _file; save(); }

    void setBool(const std::string &name, bool b);
    bool getBool(const std::string &name, bool def=false);

    void setInt(const std::string &name, int b);
    int getInt(const std::string &name, int def=0);

    // Sets 64 bit data. Uses binary encoding, since jsoncpp doesn't
    // reliably support 64 bit sizes.
    void setInt64(const std::string &name, int64_t i);
    int64_t getInt64(const std::string &name, int64_t def=0);

    // Store binary data encoded in a string
    void setData(const std::string &name, const void *p, size_t num);
    void getData(const std::string &name, void *p, size_t num);

    // Remove an entry. Non-existing entries are allowed. Returns true
    // if the entry existed before removal.
    bool remove(const std::string &name);

    // Set or get string entries
    void set(const std::string &name, const std::string &value);
    std::string get(const std::string &name, const std::string &def="");

    /* Mass set string entries. Use this if you are writing large
       amounts of data.

       JConfig rewrites the file after every single set* operation.
       This is usually OK for low-traffic config files. For some uses
       however, a file may have many thousands of entries. Rewriting a
       config file of several megabytes thousands of times is highly
       impractical.

       This function lets you set any number of entries (currently as
       strings only) in one operation, and thus only perform one
       single file write.
     */
    void setMany(const std::map<std::string,std::string> &entries);

    bool has(const std::string &name);

    std::vector<std::string> getNames();

  private:
    std::string file;
    struct _JConfig_Hidden;
    _JConfig_Hidden *p;
  };
}

#endif

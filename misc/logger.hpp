#ifndef __MISC_LOGGER_HPP_
#define __MISC_LOGGER_HPP_

#include <string>
#include <ostream>
#include <boost/shared_ptr.hpp>

namespace Misc
{
  class Logger
  {
    std::ostream *strm;
    std::string filename;
    boost::shared_ptr<void> ptr;

  public:
    // Set to true to write to stdout as well as to the log file
    bool print;

    Logger(const std::string &file);
    Logger(std::ostream *str);

    void operator()(const std::string &msg) { log(msg); }
    void log(const std::string &msg);
  };

  typedef boost::shared_ptr<Logger> LogPtr;
}

#endif

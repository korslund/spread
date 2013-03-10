#include "logger.hpp"
#include <fstream>
#include <boost/filesystem.hpp>

using namespace Misc;
namespace bf = boost::filesystem;

Logger::Logger(const std::string &file)
  : filename(file), print(false)
{
  if(bf::exists(file))
    {
      std::string old = file + ".old";
      if(bf::exists(old))
        bf::remove(old);
      bf::rename(file, old);
    }
  strm = new std::ofstream(file.c_str());
  ptr.reset(strm);
}

Logger::Logger(std::ostream *str)
  : strm(str), filename(""), print(false)
{}

Logger::Logger()
  : strm(NULL), filename(""), print(true)
{}

void Logger::log(const std::string &msg)
{
  if(print)
    {
      if(filename != "")
        std::cout << filename << ": ";
      else
        std::cout << "LOG: ";
      std::cout << msg << "\n";
    }

  if(!strm) return;
  char buf[100];
  time_t now = std::time(NULL);
  std::strftime(buf, 100, "%Y-%m-%d %H:%M:%S", gmtime(&now));
  (*strm) << buf << ":   " << msg << "\n";
  // Flush after each line in case of crashes
  strm->flush();
}


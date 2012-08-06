// This is a dummy file

#include <iostream>

namespace Spread
{
  struct SpreadLib
  {
    SpreadLib(const std::string &outDir, const std::string &tmpDir)
    {}

    JobInfoPtr dummy()
    {
      JobInfoPtr ptr(new JobInfo);
      ptr->setDone();
      return ptr;
    }

    JobInfoPtr updateFromURL(const std::string &channel,
                             const std::string &url)
    {
      using namespace std;
      cout << "Initialized channel '" << channel << " from " << url << endl;
      return dummy();
    }

    JobInfoPtr install(const std::string &channel,
                       const std::string &package,
                       const std::string &where)
    {
      using namespace std;
      cout << "Installing " << channel << "::" << package << " to " << where << endl;
      return dummy();
    }

  };
};

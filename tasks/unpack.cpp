#include "unpack.hpp"
#include <unpackcpp/auto.hpp>

using namespace Spread;

struct UProgress : UnpackCpp::Progress
{
  JobInfoPtr info;

  bool progress(int64_t total, int64_t now)
  {
    info->setProgress(now, total);

    // Abort the download if the user requested it.
    if(info->checkStatus())
      return false;

    return true;
  }
};

void UnpackTask::doJob()
{
  UProgress prog;
  prog.info = info;

  UnpackCpp::AutoUnpack unp;

  if(dir != "")
    {
      setBusy("Unpacking " + file + " to " + dir);
      unp.unpack(file, dir, &prog, list);
    }
  else
    {
      setBusy("Unpacking " + file);
      unp.unpack(file, writeTo, &prog, list);

      // Make sure the directory writer is destructed before we
      // finish. This is sort of hackish to rely resetting the smart
      // pointer, but it allows us to do the necessary cleanup that
      // some dir writers require. It is also a better solution than
      // an explicit finish() function, because this doesn't prevent
      // us from using the dir writer further if we want to.
      writeTo.reset();
    }

  // All error handling is done through exceptions. If we get here,
  // everything is OK.
  setDone();
};

#include "copyhash.hpp"
#include <mangle/stream/servers/file_stream.hpp>
#include <mangle/stream/clients/copy_stream.hpp>

using namespace Spread;
using namespace Mangle::Stream;

// This just reads from a file and dumps the data into a stream
struct CopyTask : Jobify::Job
{
  CopyTask(const std::string &input, StreamPtr output)
    : in(input), out(output) {}

  void doJob()
  {
    setBusy("Copying " + in);
    CopyStream::copy(FileStream::Open(in), out);
    setDone();
  }

private:
  std::string in;
  StreamPtr out;
};

Jobify::Job* CopyHash::createJob()
{
  // Guess the hash from targets
  assert(outputs.size() > 0);
  const Hash &hash = outputs.begin()->first;
  return new CopyTask(source, getOutStream(hash));
}

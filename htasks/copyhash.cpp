#include "copyhash.hpp"
#include <mangle/stream/servers/file_stream.hpp>
#include <mangle/stream/clients/copy_stream.hpp>

using namespace Spread;
using namespace Mangle::Stream;

// This just reads from a file and dumps the data into a stream
struct CopyTask : Job
{
  CopyTask(const std::string &input, StreamPtr output, size_t s)
    : in(input), out(output), size(s) {}

  void doJob()
  {
    setBusy("Copying " + in);
    assert(out);
    setProgress(0,size);
    size_t cpy = CopyStream::copy(FileStream::Open(in), out);
    setProgress(cpy,size);
    if(cpy != size)
      setError("Size mismatch when copying " + in);
    else
      setDone();
  }

private:
  std::string in;
  StreamPtr out;
  size_t size;
};

Job* CopyHash::createJob()
{
  desc = "copying " + source;

  // Guess the hash from targets
  assert(outputs.size() > 0);
  assert(source != "");
  const Hash &hash = outputs.begin()->first;
  assert(!hash.isNull());
  return new CopyTask(source, getOutStream(hash), hash.size());
}

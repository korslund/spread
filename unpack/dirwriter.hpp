#ifndef __UNPACK_DIRWRITER_HPP
#define __UNPACK_DIRWRITER_HPP

#include <mangle/vfs/stream_factory.hpp>

namespace Unpack
{
  /* A stream factory that writes files to a file system directory.
     Directories will be created as necessary. Fully implements all
     UnpackBase necessities:

     - calling open() on a filename returns a writable stream to the
       file path base/name.

     - calling open("") throws an exception, as does any other file
       error. The stream itself may also throw exceptions on error, of
       course.

     - all necessary nested subdirectories are created before opening
       the file

     - paths ending in / or \ are accepted. They will create the
       directory (if it doesn't already exist), then return an empty
       StreamPtr.

     - no other case returns an empty StreamPtr
  */
  struct DirWriter : Mangle::VFS::StreamFactory
  {
    DirWriter(const std::string &dir)
      : base(dir) {}

    Mangle::Stream::StreamPtr open(const std::string &name);

    std::string base;
  };
}

#endif

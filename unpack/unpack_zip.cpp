#include "unpack_zip.hpp"

#include <zzip/zzip.h>
#include <stdexcept>
#include <vector>

//#define DEBUG_PRINT
#ifdef DEBUG_PRINT
#include <iostream>
#define PRINT(a) std::cout << a << "\n";
#else
#define PRINT(a)
#endif

using namespace Unpack;
using namespace Mangle::Stream;

static void fail(const std::string &msg, const std::string &file)
{
  throw std::runtime_error("Error unpacking "+file+": " + msg);
}

static void checkErr(int err, const std::string &file)
{
  if(err != ZZIP_NO_ERROR)
    fail(zzip_strerror(err), file);
}

struct Ent
{
  std::string name;
  int64_t size;
};

void UnpackZip::unpack(const std::string &file, Mangle::VFS::StreamFactoryPtr output,
                       Progress *prog, const FileList *list)
{
  assert(output);

  PRINT("Opening ZIP " << file);

  zzip_error_t err;
  ZZIP_DIR *root = zzip_dir_open(file.c_str(), &err);
  checkErr(err, file);

  PRINT("Building ZIP index:");

  // Build a directory of all the files in the archive, and count up
  // the total size
  std::vector<Ent> dir;
  int64_t total = 0, current = 0;
  {
    ZZIP_DIRENT ent;
    while(zzip_dir_read(root, &ent))
      {
        std::string name(ent.d_name);

        PRINT("  " << name << " (" << ent.st_size << " bytes)");

        if(list && list->size())
          // Is this file on the extract list?
          if(list->count(name) == 0)
            {
              PRINT("    SKIPPED");
              // Nope. Skip it.
              continue;
            }

        Ent e;
        e.name = name;
        e.size = ent.st_size;
        dir.push_back(e);

        total += ent.st_size;
      }
    checkErr(zzip_error(root), file);
  }

  if(list && (list->size() > dir.size()))
    fail("Missing files in archive", file);

  PRINT("Extracting " << dir.size() << " elements (" << total << " bytes)");

  bool abort = false;

  // Update progress and check for abort status
  if(prog)
    abort = !prog->progress(total, current);

  for(int i=0; i<dir.size(); i++)
    {
      if(abort)
        break;

      std::string fname = dir[i].name;

      PRINT("  Opening " << fname);

      // Fetch a writable stream
      StreamPtr outs = output->open(fname);
      if(!outs) continue;
      assert(outs->isWritable);

      // Tell the stream exactly how many bytes we are going to write
      outs->reserve(dir[i].size);

      // Open the archive entry
      ZZIP_FILE *zf = zzip_file_open(root, fname.c_str(), 0);
      if(!zf)
        {
          checkErr(zzip_error(root), file);
          fail("Unknown ZIP error", file);
        }

      int64_t ftot = 0;

      while(!abort)
        {
          char buf[10*1024];
          int r = zzip_file_read(zf, buf, 10*1024);

          if(r<0)
            checkErr(zzip_error(root), file);

          outs->write(buf, r);

          current += r;
          ftot += r;

          // Update progress and check for abort status
          if(prog)
            abort = !prog->progress(total, current);

          if(r < 1024)
            break;
        }

      PRINT("  Wrote " << ftot << " bytes, expected " << dir[i].size);

      if(ftot != dir[i].size)
        fail("Output size mismatch in " + fname, file);

      zzip_file_close(zf);
      PRINT("  End of file\n");
    }

  PRINT("Closing ZIP");
  zzip_dir_close(root);
}

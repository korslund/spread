#include "hash.hpp"

using namespace Spread;

#include "../libs/sha2/sha2.h"
#include <string.h>
#include <stdexcept>
#include "../misc/comp85.hpp"

void Hash::hash(const void *input, uint32_t len)
{
  dealloc();
  sha256((const unsigned char*)input, len, data);
  size() = len;
}

void Hash::update(const void *input, uint32_t len)
{
  // Create a new context if this is the first call
  if(context == NULL)
    {
      clear();
      context = new sha256_ctx;
      sha256_init((sha256_ctx*)context);
      size() = 0;
    }

  // Update the context with the new data
  sha256_update((sha256_ctx*)context, (const unsigned char*)input, len);
  size() += len;
}

Hash Hash::finish()
{
  /* If finish is called without update() being called first, we need
     to treat it as if we are hashing a zero-length buffer.

     Finish may be preceded by any number of update() calls, including
     zero.

     Note that the hash of a zero-length buffer is NOT the same as an
     empty (zero) hash!
   */
  if(!context)
    hash(NULL, 0);
  else
    {
      sha256_ctx *ctx = (sha256_ctx*)context;

      // Finish up and delete the context
      sha256_final(ctx, data);
      dealloc();
    }

  return *this;
}

void Hash::dealloc()
{
  if(context)
    {
      sha256_ctx *ctx = (sha256_ctx*)context;
      delete ctx;
      context = NULL;
    }
}

void Hash::clear()
{
  dealloc();
  memset(data, 0, 40);
}

bool Hash::isNull() const
{
  for(int i=0; i<40; i++)
    if(data[i] != 0) return false;
  return true;
}

void Hash::copy(const void* source)
{
  dealloc();
  memcpy(data, source, 40);
}

static char hexDigit(int i)
{
  assert(i >= 0 && i < 16);
  if(i < 10) return '0' + i;
  return 'a' + i - 10;
}

static uint8_t dehexDig(char c)
{
  if(c >= '0' && c <= '9')
    return c - '0';
  if(c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  if(c >= 'A' && c <= 'F')
    return c - 'A' + 10;

  throw std::runtime_error(std::string("Invalid hex character '") + c + "'");
}

static uint8_t dehex(const char *s)
{
  return (dehexDig(*s)<<4) | dehexDig(s[1]);
}

static void fail85(const std::string &msg)
{
  throw std::runtime_error("Comp85 decode error: " + msg);
}

static Hash decode85(const std::string &hstr)
{
  Hash res;

  assert(hstr != "00");

  int len = hstr.size();

  // Check size
  if(len < 40) fail85("Hash is too short");
  if(len > 50) fail85("Hash is too long");

  /* We don't accept trailing !s in the length, as this breaks
     uniqueness. This is a SECURITY REQUIREMENT. See note in
     decode85_value().
  */
  if(len > 40 && hstr[len-1] == '!')
    fail85("Invalid trailing '!'");

  // Full input buffer
  char input[50];
  memset(input, '!', 50);
  hstr.copy(input, 50);

  // Output binary buffer
  char output[40];
  len = Comp85::decode(input, 50, &output);
  assert(len == 40);

  res.copy(output);
  assert(!res.isNull());

  return res;
}

static std::string encode85(const Hash &h)
{
  if(h.isNull()) return "00";

  const char *ptr = (const char*)h.getData();

  char output[50];

  int len = Comp85::encode(ptr, 40, output);
  assert(len == 50);

  // Trim away trailing ! chars from the length
  while(len > 40 && output[len-1] == '!')
    len--;

  return std::string(output, len);
}

std::string Hash::toHex() const
{
  std::string result;
  result.reserve(80);

  for(int i=0; i<40; i++)
    {
      uint8_t byte = data[i];
      result += hexDigit(byte >> 4);
      result += hexDigit(byte & 0xf);
    }

  return result;
}

std::string Hash::toString() const
{
  return encode85(*this);
}

void Hash::fromString(const std::string &str)
{
  clear();

  // Special encoding of null hash
  if(str == "00")
    return;

  if(str.size() >= 40 && str.size() <= 50)
    {
      *this = decode85(str);
      return;
    }

  if(str.size() != 80)
    throw std::runtime_error("String is not a valid Hash");

  const char* s = str.c_str();
  for(int i=0; i<40; i++)
    {
      data[i] = dehex(s);
      s += 2;
    }
}

std::ostream& operator<< (std::ostream& out, const Hash &hash)
{
  out << hash.toString();
  return out;
}

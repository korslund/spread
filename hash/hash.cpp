#include "hash.hpp"

using namespace Spread;

#include "../libs/sha2/sha2.h"
#include <string.h>
#include <stdexcept>

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

/* Base64 characters. Notice that we use an URL-friendly variant with
   -_ at the end instead of +/. But our decoding routine supports both.
 */
static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

// Converts 3 input bytes to 4 output characters
void toBase(const uint8_t *in, char *out)
{
  out[0] = cb64[ in[0] >> 2 ];
  out[1] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
  out[2] = cb64[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ];
  out[3] = cb64[ in[2] & 0x3f ];
}

// Decode character
uint8_t bval(char c)
{
  if(c >= 'A' && c <= 'Z') return c-'A';
  if(c >= 'a' && c <= 'z') return c-'a'+26;
  if(c >= '0' && c <= '9') return c-'0'+52;
  if(c == '-' || c == '+') return 62;
  if(c == '/' || c == '_') return 63;
  if(c == '=') return 0;

  throw std::runtime_error(std::string("Invalid base64 character '") + c + "'");
}

// Converts 4 input characters to 3 output bytes
void fromBase(const char *in, uint8_t *out)
{
  uint8_t v0 = bval(in[0]);
  uint8_t v1 = bval(in[1]);
  uint8_t v2 = bval(in[2]);
  uint8_t v3 = bval(in[3]);
  out[0] = (v0 << 2) | ((v1 & 0x30) >> 4);
  out[1] = ((v1 & 0x0f) << 4) | ((v2 & 0x3c) >> 2);
  out[2] = ((v2 & 0x03) << 6) | v3;
}

static void decodeBase(const std::string &str, uint8_t *out)
{
  assert(str.size() <= 56);

  int pos = 0;
  int outleft = 40;

  while(pos < str.size() && outleft > 0)
    {
      char buf[5] = "AAAA";
      str.copy(buf, 4, pos);

      uint8_t obuf[3];
      fromBase(buf, obuf);

      if(outleft >= 3)
        memcpy(out, obuf, 3);
      else
        memcpy(out, obuf, outleft);

      outleft -= 3;
      out += 3;
      pos += 4;
    }
}

std::string Hash::toBase64() const
{
  if(isNull()) return "00";

  const uint8_t *ptr = data;

  char output[56];

  for(int i=0; i<13; i++)
    toBase(ptr+i*3, output+i*4);
  uint8_t last[3];
  last[0] = ptr[39];
  last[1] = last[2] = 0;
  toBase(last, output+52);

  // Trim trailing As
  int end = 55;
  while(output[end] == 'A' && end>0) end--;

  return std::string(output, end+1);
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
  return toBase64();
}

void Hash::fromString(const std::string &str)
{
  clear();

  // Special encoding of null hash
  if(str == "00")
    return;

  if(str.size() <= 56)
    {
      decodeBase(str, data);
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

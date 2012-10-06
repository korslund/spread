#ifndef _SPREAD_HASH_HPP
#define _SPREAD_HASH_HPP

#include <stdint.h>
#include <string>
#include <assert.h>
#include <ostream>
#include <map>

namespace Spread
{
  /* This structure represents an object hash. Most objects in Spread
     are primarily found, indexed and refered to by their hashed
     values.

     A hash is a 40 byte structure, where the first 32 bytes is the
     SHA-256 hash of the object data, and the last 8 bytes represent
     the object's size.

     Hashes can be computed in one go, or iteratively through
     update()/finish().

     Hashes are fully usable as value types, and can safely be copied
     and passed by value. They can also be used as key values in STL
     containers.

     NOTE: Copying partial hashes (where you have called update() but
     not finish()) will work, but make sure the assignment operator=
     or the copy constructor is invoked (ie. NOT a bitwise copy.)
     Otherwise results are undefined.
   */
  struct Hash
  {
    typedef std::map<std::string,Hash> DirMap;

    Hash() : context(NULL) { clear(); }
    Hash(const std::string &hex) : context(NULL) { fromString(hex); }
    Hash(const void *input, uint32_t len) : context(NULL) { hash(input, len); }
    Hash(const Hash &h) : context(NULL) { copy(h); }
    ~Hash() { dealloc(); }

    // Zeros out the hash digest
    void clear();

    // True if the digest is zeroed out
    bool isNull() const;

    // Copy digest verbatim from another source. Must be 40 bytes
    // long.
    void copy(const void* source);

    // Copy contents from another Hash. Will also copy partial hash
    // state (from update()/finish() functions), if any.
    void copy(const Hash &other);

    // Hash a complete buffer, and store the result in this struct.
    void hash(const void *input, uint32_t len);

    /* Perform and finish partial hashing. You can call update()
       sequentially on parts of your buffer. After calling finish(),
       the result will be the same as if you had called hash() on the
       entire buffer.

       There is no 'start' step. Just call update() to begin a new
       hash sequence, and finish() to get the result. The latter
       returns this struct for convenience.

       Calling update() again after finish() starts a new hash
       sequence. Calling finish() with no preceding update() step
       counts as hashing a zero-length buffer.

       Calls to clear(), copy(), hash() and fromString() will also
       abort the sequence.
    */
    void update(const void *input, uint32_t len);
    Hash finish();

    Hash operator=(const Hash &other)
    {
      copy(other);
      return *this;
    }

    // Compare two hashes for equality
    bool operator==(const Hash &other) const
    {
      for(int i=0; i<40; i++)
        if(data[i] != other.data[i])
          return false;
      return true;
    }

    bool operator!=(const Hash &other) const
    { return !(*this == other); }

    // Allows using Hash as a key for std::map
    bool operator<(const Hash &other) const
    {
      for(int i=0; i<40; i++)
        if(data[i] < other.data[i]) return true;
        else if(data[i] > other.data[i]) return false;
      return false;
    }

    // Get and set the size part of the digest
    uint64_t size() const
    {
      return *( (uint64_t*)(data+32) );
    }

    uint64_t &size()
    {
      return *( (uint64_t*)(data+32) );
    }

    // Converts hash digest to compact85 (40-50 characters long)
    std::string toString() const;

    // Converts hash digest to base64url (43-54 characters long)
    std::string toBase64() const;

    // Converts hash digest to an 80 byte long hex string
    std::string toHex() const;

    // Convert string to hash. Accepts both compact85 and hex
    // strings. The string length must represent the exact hash
    // length.
    void fromString(const std::string &str);

    // Return a pointer to the raw binary hash
    const uint8_t *getData() const { return data; }

  private:
    uint8_t data[40];

    // Internal pointer used by update() and finish()
    void *context;

    // Discard all partial hash information created by update(). This
    // has the same effect as calling finish(), except the result is
    // discarded.
    void dealloc();
  };
}

std::ostream& operator<< (std::ostream& out, const Spread::Hash &hash);

#endif

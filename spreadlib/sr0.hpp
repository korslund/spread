#ifndef __SPREADLIB_SR0_HPP_
#define __SPREADLIB_SR0_HPP_

#include <string>

/* This file is meand to implement the SR0 protocol.

   SR0 is Spread-REST v0, a very early attempt at a common and simple
   protocol for using Spread over HTTP and other transfer protocols.

   No particular care has been taken yet to make sure the protocol
   adheres to the constraints of RESTfulness(*), this more of an
   inspiration and long-term aim.

   (*) http://en.wikipedia.org/wiki/Representational_state_transfer#Constraints
 */

namespace SR0
{
  /* Updates names.json and rules.json in the given directory from the
     given SR0 file.
   */
  void applyFile(const std::string &dir, const std::string &file);

  /* Download the SR0 file from an url, then call applyFile() on it
   */
  void applyURL(const std::string &dir, const std::string &url);
};

#endif

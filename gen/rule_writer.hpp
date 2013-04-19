#ifndef __RULES_JSON_WRITER_HPP_
#define __RULES_JSON_WRITER_HPP_

#include <mangle/stream/stream.hpp>
#include "packlister.hpp"

namespace SpreadGen
{
  extern void writeRulesJson(const Spread::RuleList &rules,
                             const ArcRuleSet &arcs,
                             Mangle::Stream::StreamPtr out);
}

#endif

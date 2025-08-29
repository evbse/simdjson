#ifndef SIMDJSON_FALLBACK_STRINGPARSING_DEFS_H
#define SIMDJSON_FALLBACK_STRINGPARSING_DEFS_H

#ifndef SIMDJSON_CONDITIONAL_INCLUDE
#include "simdjson/fallback/base.h"
#endif // SIMDJSON_CONDITIONAL_INCLUDE

namespace simdjson {
namespace fallback {
namespace {

// Holds backslashes and quotes locations.
struct backslash_and_quote {
public:
  static constexpr uint32_t BYTES_PROCESSED = 8;
  simdjson_inline static backslash_and_quote copy_and_find(const uint8_t *src, uint8_t *dst);

  simdjson_inline bool has_quote_first() { return qt_first; }
  simdjson_inline bool has_backslash() { return bs_first; }
  simdjson_inline int quote_index() { return qt_index; }
  simdjson_inline int backslash_index() { return bs_index; }

  bool qt_first;
  int qt_index;
  bool bs_first;
  int bs_index;

}; // struct backslash_and_quote

simdjson_inline backslash_and_quote backslash_and_quote::copy_and_find(const uint8_t *src, uint8_t *dst) {
  // store to dest unconditionally - we can overwrite the bits we don't like later
  uint64_t c;
  std::memcpy(&c, src, 8);
  std::memcpy(dst, src, 8);

  uint64_t QUOTE = 0x2222222222222222;
  uint64_t BACKSLASH = 0x5c5c5c5c5c5c5c5c;

  uint64_t qt = c ^ QUOTE;
  uint64_t bs = c ^ BACKSLASH;
  uint64_t has_zero = ((qt - 0x0101010101010101) & ~qt & 0x8080808080808080) |
                      ((bs - 0x0101010101010101) & ~bs & 0x8080808080808080);

  if (has_zero) {
    for (int i = 0; i < 8; i += 1) {
      if (src[i] == '"')
        return { true, i, false, -1 };
      if (src[i] == '\\')
        return { false, -1, true, i };
    }
  }

  return { false, -1, false, -1 };
}


struct escaping {
  static constexpr uint32_t BYTES_PROCESSED = 1;
  simdjson_inline static escaping copy_and_find(const uint8_t *src, uint8_t *dst);

  simdjson_inline bool has_escape() { return escape_bits; }
  simdjson_inline int escape_index() { return 0; }

  bool escape_bits;
}; // struct escaping



simdjson_inline escaping escaping::copy_and_find(const uint8_t *src, uint8_t *dst) {
  dst[0] = src[0];
  return { (src[0] == '\\') || (src[0] == '"') || (src[0] < 32) };
}

} // unnamed namespace
} // namespace fallback
} // namespace simdjson

#endif // SIMDJSON_FALLBACK_STRINGPARSING_DEFS_H

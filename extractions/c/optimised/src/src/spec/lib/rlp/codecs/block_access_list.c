#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

Bytes bal_ref_cursor(struct RlpFieldRef f)
{
  bool framing_canonical = rlp_ref_framing_canonical(f);
  if (f.is_list && framing_canonical) {
    return rlp_decode_list(f);
  }
  fatal_error(InvalidBlockAccessList);
}

u256 bal_ref_word(struct RlpFieldRef f)
{
  bool canonical = rlp_item_uint_canonical(f);
  if (canonical && (f.content_len <= UINT8_C(32))) {
    return rlp_decode_word(f);
  }
  fatal_error(InvalidBlockAccessList);
}

Bytes bal_ref_bytes(struct RlpFieldRef f)
{
  if (rlp_ref_bytes_canonical(f)) {
    return rlp_item_content(f);
  }
  fatal_error(InvalidBlockAccessList);
}

uint64_t bal_ref_uint64(struct RlpFieldRef f)
{
  bool canonical = rlp_item_uint_canonical(f);
  if (canonical && (f.content_len <= UINT8_C(8))) {
    return rlp_decode_uint64(f);
  }
  fatal_error(InvalidBlockAccessList);
}

void bal_expect_end(Bytes cursor)
{
  if (cursor.len == UINT8_C(0)) {
    return;
  }
  fatal_error(InvalidBlockAccessList);
}


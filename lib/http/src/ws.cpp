#include "ws.hpp"

std::vector<u8> get_ws(FrameHeader const &h) {
  u32 mask_offset;
  u32 size;
  if (h.SIZE == 126) {
    mask_offset = 4;
    size = (u64(h.SIZE_BYTES[0]) << 8) | (u64(h.SIZE_BYTES[1]) << 0);
  } else if (h.SIZE == 127) {
    mask_offset = 10;
    size = (u64(h.SIZE_BYTES[0]) << 56) | (u64(h.SIZE_BYTES[1]) << 48) |
           (u64(h.SIZE_BYTES[2]) << 40) | (u64(h.SIZE_BYTES[3]) << 32) |
           (u64(h.SIZE_BYTES[4]) << 24) | (u64(h.SIZE_BYTES[5]) << 16) |
           (u64(h.SIZE_BYTES[6]) << 8) | (u64(h.SIZE_BYTES[7]) << 0);
  } else {
    mask_offset = 2;
    size = u64(h.SIZE);
  }
  u32 masking_key = *(u32 *)(((char *)&h) + mask_offset);

  std::vector<u8> DATA = {(u8 *)&h + mask_offset + 4,
                          (u8 *)&h + mask_offset + 4 + size};

  u32 i = 0;
  for (auto &b : DATA) {
    b = b ^ ((masking_key >> (i * 8)) & 0xff);
    i++;
    if (i >= 4)
      i -= 4;
  }

  return DATA;
}

std::vector<u8> write_ws(std::vector<u8> const &data) {
  u64 s = data.size();
  u32 size_off = 0;
  u8 size_byte = 0;
  if (s < 125) {
    size_byte = u8(s);
  } else if (s < 65536) {
    size_byte = 126;
    size_off = 2;
  } else {
    size_byte = 127;
    size_off = 8;
  }
  std::vector<u8> out(2 + size_off);
  out.insert(out.end(), data.begin(), data.end());
  out[0] = 0b10000001;
  out[1] = size_byte;
  if (size_off == 2) {
    out[2] = (s >> 8) & 0xff;
    out[3] = (s >> 0) & 0xff;
  } else if (size_off == 8) {
    out[2] = (s >> 56) & 0xff;
    out[3] = (s >> 48) & 0xff;
    out[4] = (s >> 40) & 0xff;
    out[5] = (s >> 32) & 0xff;
    out[6] = (s >> 24) & 0xff;
    out[7] = (s >> 16) & 0xff;
    out[8] = (s >> 8) & 0xff;
    out[9] = (s >> 0) & 0xff;
  }

  return out;
}

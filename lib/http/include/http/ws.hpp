#pragma once
#include <defines.hpp>
#include <vector>

struct FrameHeader {
  u8 OPCODE : 4;
  u8 RSV1 : 1;
  u8 RSV2 : 1;
  u8 RSV3 : 1;
  u8 FIN : 1;
  u8 SIZE : 7;
  u8 MASK : 1;
  u8 SIZE_BYTES[8];
};

std::vector<u8> get_ws(FrameHeader const &h);
std::vector<u8> write_ws(std::vector<u8> const &data);

#ifndef CODEC_H_INCLUDED
#define CODEC_H_INCLUDED

#include <stdint.h>
#include <optional>
#include <string>
#include <string_view>

std::optional<uint64_t> DecodeWalls(std::string_view s);

std::string EncodeWalls(uint64_t walls_bitmask);

int ParseMove(std::string_view s);

std::string FormatMove(int i);

#endif // ndef CODEC_H_INCLUDED

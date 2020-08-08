#include "codec.h"

#include <assert.h>

namespace {

constexpr char BASE64_DIGITS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

int DecodeBase64(char ch) {
    if (ch >= 'A' && ch <= 'Z') return ch - 'A';
    if (ch >= 'a' && ch <= 'z') return ch - 'a' + 26;
    if (ch >= '0' && ch <= '9') return ch - '0' + 52;
    if (ch == '-') return 62;
    if (ch == '_') return 63;
    return -1;
}

}  // namespace

std::optional<uint64_t> DecodeWalls(std::string_view s) {
    if (s.size() != 10) return std::nullopt;
    uint64_t walls_bitmask = 0;
    for (int i = 0; i < 10; ++i) {
        int word = DecodeBase64(s[i]);
        if (word < 0) return std::nullopt;
        for (int j = 0; j < 6; ++j) {
            if ((word & (1 << j)) != 0) {
                walls_bitmask |= uint64_t{1} << (6*i + j);
            }
        }
    }
    return walls_bitmask;
}

std::string EncodeWalls(uint64_t walls_bitmask) {
    std::string encoded(10, '\0');
    for (int i = 0; i < 10; ++i) {
        encoded[i] = BASE64_DIGITS[(walls_bitmask >> (6 * i)) & 63];
    }
    return encoded;
}

int ParseMove(std::string_view s) {
    if (s.size() != 3) return -1;
    int row = int{s[0]} - 'A';
    int col = int{s[1]} - '1';
    if (row < 0 || col < 0) return -1;
    if (s[2] == 'h') {
        if (col >= 5) return -1;
        return 11*row + col;
    } else if (s[2] == 'v') {
        if (col >= 6) return -1;
        return 11*row + 5 + col;
    } else {
        return -1;
    }
}

std::string FormatMove(int i) {
    assert(i >= 0 && i < 60);
    int row = i / 11;
    int col = i % 11;
    char dir;
    if (col < 5) {
        dir = 'h';
    } else {
        col -= 5;
        dir = 'v';
    }
    std::string res;
    res += static_cast<char>('A' + row);
    res += static_cast<int>('1' + col);
    res += dir;
    return res;
}

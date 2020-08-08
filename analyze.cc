#include "analysis.h"
#include "codec.h"

#include <iostream>

int main(int argc, char *argv[]) {
    if (argc != 2)  {
        std::cout << "Usage: analyze [state]" << std::endl;
        return 1;
    }
    const char *encoded_walls = argv[1];
    std::optional<uint64_t> walls_bitmask = DecodeWalls(encoded_walls);
    if (!walls_bitmask) {
        std::cerr << "Failed to decode state [" << encoded_walls << "]\n";
        return 1;
    }
    WriteAsciiWalls(*walls_bitmask, std::cerr);
    WriteDotGraph(*walls_bitmask, std::cout);
    Analyze(*walls_bitmask);
    return 0;
}

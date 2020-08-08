#include "analysis.h"
#include "codec.h"

#include <assert.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#include <iostream>
#include <utility>
#include <vector>

namespace {

int SelectMove(uint64_t walls_bitmask) {
    std::vector<int> moves = ListValidMoves(walls_bitmask);
    return moves.empty() ? -1 : moves[random() % moves.size()];
}

bool IsGameOver(uint64_t walls_bitmask) {
    return ListValidMoves(walls_bitmask).empty();
}

}  // namespace

int main(int argc, char* argv[]) {
    srandom(time(NULL) - 31337 * getpid());

    uint64_t walls = 0;
    std::string line;
    while (std::getline(std::cin, line)) {
        int move;
        if (line == "Start") {
            std::cerr << "Received Start." << std::endl;
        } else if (line == "Quit") {
            std::cerr << "Received Quit." << std::endl;
            return 0;
        } else if (move = ParseMove(line), move >= 0) {
            std::cerr << "Received move: " << line << std::endl;
            walls |= uint64_t{1} << move;
            if (IsGameOver(walls)) {
                std::cerr << "Game is over! I lost :-(" << std::endl;
                return 0;
            }
        } else {
            std::cerr << "Received invalid line: " << line << "! Quitting." << std::endl;
            return 1;
        }
        std::cerr << "State: " << EncodeWalls(walls) << std::endl;
        move = SelectMove(walls);
        assert(move >= 0);
        walls |= uint64_t{1} << move;
        std::string move_string = FormatMove(move);
        std::cerr << "Sent move: " << move_string << std::endl;
        std::cout << move_string << std::endl;
        if (IsGameOver(walls)) {
            std::cerr << "Game is over! I won :-)" << std::endl;
            return 0;
        }
    }
    std::cerr << "Premature end of input! Quitting.\n";
    return 1;
}

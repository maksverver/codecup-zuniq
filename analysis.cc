#include "analysis.h"

#include <assert.h>
#include <string.h>

#include <set>
#include <optional>
#include <iostream>

using namespace std;

#define FOR(i, a, b) for (int i = a, _##i##_end_ = b; i != _##i##_end_; ++i)
#define REP(i, n) FOR(i, 0, n)

#define CHECK(x) if (!(x)) abort();

namespace {

constexpr int H = 5;
constexpr int W = 5;
constexpr int STRIDE = W + (W + 1);
constexpr int NUM_MOVES = H * (W + 1) + W * (H + 1);

struct Walls {
    uint64_t walls_bitmask;

    inline bool Top(int r, int c) const { return (walls_bitmask & (uint64_t{1} << (STRIDE*r + c))) != 0; }
    inline bool Left(int r, int c) const{ return (walls_bitmask & (uint64_t{1} << (STRIDE*r + W + c))) != 0; }
    inline bool Bottom(int r, int c) const { return Top(r + 1, c); }
    inline bool Right(int r, int c) const { return Left(r, c + 1); }
};

struct Dfs {
    Walls walls;
    int num_groups = 0;
    int num_visited = 0;
    int label[H][W];  // node label, or -1 if not visited yet
    int group[H][W];  // group identifier, or -1 if not visited yet
    int parent[H][W];  // node from which this one was visited, 0 for outside, -1 if not visited yet
    int closed[H][W];  // size of the closed region this cell belongs to, or 0 if it is not part of a closed region
    std::set<int> sizes_used;

    Dfs(uint64_t walls_bitmask) : walls(Walls{walls_bitmask}) {
        REP(r, H) REP(c, W) label[r][c] = -1;
        REP(r, H) REP(c, W) group[r][c] = -1;
        REP(r, H) REP(c, W) parent[r][c] = -1;
        REP(r, H) REP(c, W) closed[r][c] = -1;
    }

    void Visit(int r, int c, int p, int g) {
        assert(r >= 0 && c >= 0 && r < H && c < W);
        int q = ++num_visited;
        assert(label[r][c] == -1);
        label[r][c] = q;
        parent[r][c] = p;
        group[r][c] = g;
        closed[r][c] = 0;
        if (r > 0 && !walls.Top(r, c) && label[r - 1][c] < 0) Visit(r - 1, c, q, g);
        if (c > 0 && !walls.Left(r, c) && label[r][c - 1] < 0) Visit(r, c - 1, q, g);
        if (r + 1 < H && !walls.Bottom(r, c) && label[r + 1][c] < 0) Visit(r + 1, c, q, g);
        if (c + 1 < W && !walls.Right(r, c) && label[r][c + 1] < 0) Visit(r, c + 1, q, g);
    }

    bool Explore() {
        REP(c, W) {
            if (!walls.Top(0, c) && label[0][c] < 0) Visit(0, c, 0, num_groups++);
            if (!walls.Bottom(H - 1, c) && label[H - 1][c] < 0) Visit(H - 1, c, 0, num_groups++);
        }
        REP(r, H) {
            if (!walls.Left(r, 0) && label[r][0] < 0) Visit(r, 0, 0, num_groups++);
            if (!walls.Right(r, W - 1) && label[r][W - 1] < 0) Visit(r, W - 1, 0, num_groups++);
        }
        REP(r, H) REP(c, W) if (label[r][c] < 0) {
            int g = num_groups++;
            int n = num_visited;
            Visit(r, c, n, g);
            int size = num_visited - n;
            if (!sizes_used.insert(size).second) return false;
            REP(rr, H) REP(cc, W) if (group[rr][cc] == g) closed[rr][cc] = size;
        }
        return true;
    }
};

}  // namespace

void WriteAsciiWalls(uint64_t walls_bitmask, std::ostream &os) {
    Walls walls{walls_bitmask};
    Dfs dfs(walls_bitmask);
    CHECK(dfs.Explore());
    for (int r = 0; r <= H; ++r) {
        for (int c = 0; c <= W; ++c) {
            os << '+';
            if (c < W) {
                os << (walls.Top(r, c) ? "--" : "  ");
            }
        }
        os << '\n';
        if (r < H) {
            for (int c = 0; c <= W; ++c) {
                os << (walls.Left(r, c) ? '|' : ' ');
                if (c < W) {
                    int size = dfs.closed[r][c];
                    if (size > 9) {
                        os << size;
                    } else if (size > 0) {
                        os << ' ' << size;
                    } else {
                        //os << ' ' << char{'a' + dfs.group[r][c]};
                        os << "  ";
                    }
                }
            }
        }
        os << '\n';
    }
}

void WriteDotGraph(uint64_t walls_bitmask, std::ostream &os) {
    Walls walls{walls_bitmask};
    Dfs dfs(walls_bitmask);
    CHECK(dfs.Explore());
    os << "graph {\n";
    REP(g, dfs.num_groups) {
        REP(r, H) REP(c, W) {
            int i = dfs.label[r][c];
            assert(i >= 0);
            if (dfs.group[r][c] != g || dfs.closed[r][c]) continue;
            os << "  g" << g << "v0 [label=\"0\", style=\"filled\"]\n";
            os << "  v" << i << " [label=\"" << i << "\\n(" << r << ',' << c << ")\"]\n";
            // Note: a double edge to 0 is possible (for corner squares)!
            if (!walls.Top(r, c)) {
                if (r > 0) {
                    os << "  v"  << dfs.label[r - 1][c] << " -- v" << i << ";\n";
                } else {
                    os << "  g" << g << "v0 -- v" << i << ";\n";
                }
            }
            if (!walls.Left(r, c)) {
                if (c > 0) {
                    os << "  v"  << dfs.label[r][c - 1] << " -- v" << i << ";\n";
                } else {
                    os << "  g" << g << "v0 -- v" << i << ";\n";
                }
            }
            if (!walls.Bottom(r, c)) {
                if (r < H - 1) {
                    //
                } else {
                    os << "  g" << g << "v0 -- v" << i << ";\n";
                }
            }
            if (!walls.Right(r, c)) {
                if (c < W - 1) {
                    //
                } else {
                    os << "  g" << g << "v0 -- v" << i << ";\n";
                }
            }
        }
    }
    os << "}\n";
}

bool IsValidState(uint64_t walls_bitmask) {
    return Dfs(walls_bitmask).Explore();
}

std::vector<int> ListValidMoves(uint64_t walls_bitmask) {
    Dfs dfs(walls_bitmask);
    CHECK(dfs.Explore());

    std::vector<int> valid_moves;
    REP(i, NUM_MOVES) {
        // Rule 1: place a wall in an open space.
        uint64_t bit = uint64_t{1} << i;
        if ((walls_bitmask & bit) != 0) continue;

        // Rule 2: can't place a wall in a closed region.
        int r = i / STRIDE;
        int c = i % STRIDE;
        if (c > W) c -= W;
        if (r < H && c < W && dfs.closed[r][c] != 0) continue;

        // Rule 3: can't create two regions of the same size.
        if (!IsValidState(walls_bitmask | bit)) continue;

        valid_moves.push_back(i);
    }
    return valid_moves;
}

void Analyze(uint64_t walls_bitmask) {
    if (!IsValidState(walls_bitmask)) {
        std::cerr << "State is invalid!\n";
        return;
    }
    std::cerr << "Moves:";
    for (int i : ListValidMoves(walls_bitmask)) {
        std::cerr << ' ' << i;
    }
    std::cerr << std::endl;

    // convert to graph
    // identify bridges and sizes of subgraphs cut.
    // bridges necessarily form a tree?
}

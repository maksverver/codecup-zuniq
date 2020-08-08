#ifndef ANALYSIS_H_INCLUDED
#define ANALYSIS_H_INCLUDED

#include <cstdint>
#include <iostream>
#include <vector>

bool IsValidState(uint64_t walls_bitmask);
std::vector<int> ListValidMoves(uint64_t walls_bitmask);

void WriteAsciiWalls(uint64_t walls_bitmask, std::ostream &os);
void WriteDotGraph(uint64_t walls_bitmask, std::ostream &os);
void Analyze(uint64_t walls_bitmask);

#endif  // ndef ANALYSIS_H_INCLUDED

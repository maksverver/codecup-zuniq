CXXFLAGS=-Wall -pipe -O2 -march=native -g -std=c++17
LDLIBS=-lm

PLAYER_SRCS=analysis.cc codec.cc player.cc
PLAYER_HDRS=analysis.h codec.h

ANALYZE_SRCS=analysis.cc codec.cc analyze.cc
ANALYZE_HDRS=analysis.h codec.h

all: player analyze

clean:

distclean: clean
	rm -f player analyze

analyze: $(ANALYZE_HDRS) $(ANALYZE_SRCS)
	$(CXX) -o $@ $(CXXFLAGS) $(ANALYZE_SRCS) $(LDLIBS)

player: $(PLAYER_HDRS) $(PLAYER_SRCS)
	$(CXX) -o $@ $(CXXFLAGS) $(PLAYER_SRCS) $(LDLIBS)

.PHONY: all clean distclean

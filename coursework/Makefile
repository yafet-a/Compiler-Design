CXX = clang++ -std=c++17
CFLAGS = -g -O3 `llvm-config --cppflags --ldflags --system-libs --libs all` \
-Wno-unused-function -Wno-unknown-warning-option -fno-rtti

SRC_DIR = src
INCLUDE_DIR = include

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)

mccomp: $(SOURCES)
	$(CXX) $(SOURCES) $(CFLAGS) -I$(INCLUDE_DIR) -o mccomp

clean:
	rm -rf mccomp
// Clock.h
#pragma once

#include <vector>

// Simple Clock page replacement algorithm.
// We only track frame numbers and reference bits;
// the VirtualMemory layer decides which page maps to which frame.
class Clock {
private:
    int capacity;                  // number of frames
    std::vector<int> frames;       // frame -> pageNumber (or -1 if empty)
    std::vector<int> refBits;      // reference bits for each frame
    int hand;                      // current clock hand position

public:
    explicit Clock(int capacity);

    // Returns index of frame where the new page is placed.
    // If free frame exists, use it; otherwise evict according to Clock.
    int selectFrame(int pageNumber);

    // Mark that a given frame was referenced (set use bit).
    void referenceFrame(int frameIndex);

    // For debugging
    void printState() const;
};

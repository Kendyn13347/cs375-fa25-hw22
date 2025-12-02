// Clock.cpp
#include "Clock.h"
#include <iostream>

Clock::Clock(int capacity_)
    : capacity(capacity_),
      frames(capacity_, -1),
      refBits(capacity_, 0),
      hand(0) {}

int Clock::selectFrame(int pageNumber) {
    // First, look for a free frame.
    for (int i = 0; i < capacity; ++i) {
        if (frames[i] == -1) {
            frames[i] = pageNumber;
            refBits[i] = 1;
            return i;
        }
    }

    // No free frame: run the clock algorithm
    while (true) {
        if (refBits[hand] == 0) {
            // evict this frame
            frames[hand] = pageNumber;
            refBits[hand] = 1;
            int chosen = hand;
            hand = (hand + 1) % capacity;
            return chosen;
        } else {
            // give a second chance
            refBits[hand] = 0;
            hand = (hand + 1) % capacity;
        }
    }
}

void Clock::referenceFrame(int frameIndex) {
    if (frameIndex >= 0 && frameIndex < capacity) {
        refBits[frameIndex] = 1;
    }
}

void Clock::printState() const {
    std::cout << "Clock frames: ";
    for (int i = 0; i < capacity; ++i) {
        std::cout << "[" << i << ":" << frames[i]
                  << ",use=" << refBits[i] << "] ";
    }
    std::cout << "\n";
}

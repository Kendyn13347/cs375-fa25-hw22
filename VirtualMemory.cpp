// VirtualMemory.cpp
#include "VirtualMemory.h"
#include <iostream>
#include <algorithm>
#include <cmath>

VirtualMemory::VirtualMemory(int tlbSize_, int pageCount_)
    : tlbSize(tlbSize_),
      currentTime(0),
      pageCount(pageCount_),
      tlbHits(0),
      tlbMisses(0),
      pageTableHits(0),
      pageFaults(0)
{
    if (tlbSize <= 0) tlbSize = 1;
    tlb.resize(tlbSize);

    // for a simple split, make the outer have ~sqrt(pageCount) entries
    if (pageCount <= 0) pageCount = 1;
    pagesPerOuter = std::max(1, static_cast<int>(std::sqrt(pageCount)));
}

void VirtualMemory::splitPageNumber(int pageNumber,
                                    int &outerIdx,
                                    int &innerIdx) const {
    outerIdx = pageNumber / pagesPerOuter;
    innerIdx = pageNumber % pagesPerOuter;
}

// Look up PID + VA in the TLB
bool VirtualMemory::lookupTLB(int pid, int virtualAddress,
                              int &physicalAddress) {
    int page = getPageNumber(virtualAddress);
    int offset = getOffset(virtualAddress);

    ++currentTime;
    for (auto &entry : tlb) {
        if (entry.pid == pid && entry.pageNumber == page) {
            tlbHits++;
            entry.lastUsed = currentTime;
            physicalAddress = entry.frameNumber * PAGE_SIZE + offset;
            return true;
        }
    }
    tlbMisses++;
    return false;
}

// Insert/replace entry in TLB using LRU
void VirtualMemory::insertTLB(int pid, int pageNumber, int frameNumber) {
    ++currentTime;

    // try empty slot first
    for (auto &entry : tlb) {
        if (entry.pid == -1) {
            entry.pid = pid;
            entry.pageNumber = pageNumber;
            entry.frameNumber = frameNumber;
            entry.lastUsed = currentTime;
            return;
        }
    }

    // otherwise evict least recently used
    auto victimIt = std::min_element(
        tlb.begin(), tlb.end(),
        [](const TLBEntry &a, const TLBEntry &b) {
            return a.lastUsed < b.lastUsed;
        });

    victimIt->pid = pid;
    victimIt->pageNumber = pageNumber;
    victimIt->frameNumber = frameNumber;
    victimIt->lastUsed = currentTime;
}

// Mark that (pid,pageNumber) is loaded into frameNumber
void VirtualMemory::loadPage(int pid, int pageNumber, int frameNumber) {
    int outerIdx, innerIdx;
    splitPageNumber(pageNumber, outerIdx, innerIdx);

    outerPageTable[{pid, outerIdx}] = 1;               // mark inner table present
    innerPageTable[{pid, pageNumber}] = frameNumber;   // map vpage -> frame
}

// Full translation: use TLB first, then two-level page table.
// Return true if successful (page in memory), false on page fault.
bool VirtualMemory::translateAddress(int pid,
                                     int virtualAddress,
                                     int &physicalAddress) {
    // 1. Check TLB
    if (lookupTLB(pid, virtualAddress, physicalAddress)) {
        return true;
    }

    int page = getPageNumber(virtualAddress);
    int offset = getOffset(virtualAddress);

    int outerIdx, innerIdx;
    splitPageNumber(page, outerIdx, innerIdx);

    // 2. Outer table: does the inner table exist?
    auto outerIt = outerPageTable.find({pid, outerIdx});
    if (outerIt == outerPageTable.end()) {
        // no inner table: page fault
        pageFaults++;
        return false;
    }

    // 3. Inner table: does this page map to a frame?
    auto innerIt = innerPageTable.find({pid, page});
    if (innerIt == innerPageTable.end()) {
        // page fault
        pageFaults++;
        return false;
    }

    pageTableHits++;
    int frame = innerIt->second;
    physicalAddress = frame * PAGE_SIZE + offset;

    // 4. Update TLB
    insertTLB(pid, page, frame);

    return true;
}

void VirtualMemory::printStats() const {
    std::cout << "=== Virtual Memory Statistics ===\n";
    std::cout << "TLB hits:        " << tlbHits << "\n";
    std::cout << "TLB misses:      " << tlbMisses << "\n";
    std::cout << "Page table hits: " << pageTableHits << "\n";
    std::cout << "Page faults:     " << pageFaults << "\n";
}

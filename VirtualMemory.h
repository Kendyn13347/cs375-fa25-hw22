// VirtualMemory.h
#pragma once

#include <map>
#include <vector>
#include <cstdint>

// One entry in the TLB
class TLBEntry {
public:
    int pid;
    int pageNumber;   // virtual page #
    int frameNumber;  // physical frame #
    int lastUsed;     // for LRU in the TLB

    TLBEntry()
        : pid(-1), pageNumber(-1), frameNumber(-1), lastUsed(0) {}
};

class VirtualMemory {
private:
    // ---- TLB ----
    std::vector<TLBEntry> tlb;          // fixed-size TLB
    int tlbSize;
    int currentTime;                    // logical time for LRU

    // ---- Two-level page table ----
    // outer: (pid, outerPage) -> 1 (just marks that the inner table exists)
    std::map<std::pair<int,int>, int> outerPageTable;

    // inner: (pid, virtualPage) -> frame#
    // (we model the “inner index” by the full virtual page number
    //  but still keep an outer table to mimic two-level structure)
    std::map<std::pair<int,int>, int> innerPageTable;

    // Parameters for page split
    int pageCount;          // total # virtual pages per process
    int pagesPerOuter;      // how many pages per outer entry

    // ---- Statistics ----
    int tlbHits;
    int tlbMisses;
    int pageTableHits;
    int pageFaults;

    // Helper: break virtual address into page# and offset
    static constexpr int PAGE_SIZE = 4096;       // 4KB
    static int getPageNumber(int virtualAddress) {
        return virtualAddress / PAGE_SIZE;
    }
    static int getOffset(int virtualAddress) {
        return virtualAddress % PAGE_SIZE;
    }

    // Helper: split page into outer + inner index
    void splitPageNumber(int pageNumber, int &outerIdx, int &innerIdx) const;

public:
    VirtualMemory(int tlbSize, int pageCount);

    // Task 1 API
    bool lookupTLB(int pid, int virtualAddress, int &physicalAddress);
    bool translateAddress(int pid, int virtualAddress, int &physicalAddress);
    void insertTLB(int pid, int pageNumber, int frameNumber);
    void loadPage(int pid, int pageNumber, int frameNumber);
    void printStats() const;
};

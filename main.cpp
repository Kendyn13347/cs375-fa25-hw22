// main.cpp
#include "VirtualMemory.h"
#include "Clock.h"

#include <fstream>
#include <iostream>
#include <sstream>

int main() {
    // Parameters – adjust to match your assignment
    const int TLB_SIZE     = 8;
    const int PAGE_COUNT   = 256;   // virtual pages per process
    const int NUM_FRAMES   = 32;    // total physical frames

    VirtualMemory vm(TLB_SIZE, PAGE_COUNT);
    Clock clockAlg(NUM_FRAMES);

    std::ifstream in("input/trace.txt");
    if (!in) {
        std::cerr << "Failed to open input/trace.txt\n";
        return 1;
    }

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        int pid, vaddr;
        if (!(iss >> pid >> vaddr)) continue;

        int paddr;
        if (vm.translateAddress(pid, vaddr, paddr)) {
            // Access succeeded, tell Clock that frame was used
            int frame = paddr / 4096;
            clockAlg.referenceFrame(frame);
            // For debugging:
            // std::cout << "PID " << pid << " VA " << vaddr
            //           << " -> PA " << paddr << "\n";
        } else {
            // Page fault: choose frame by Clock, load page, retry
            int page = vaddr / 4096;
            int frame = clockAlg.selectFrame(page);
            vm.loadPage(pid, page, frame);

            // Now the translation should succeed
            vm.translateAddress(pid, vaddr, paddr);
            // clockAlg.referenceFrame(frame);  // already referenced in selectFrame
        }
    }

    vm.printStats();
    std::cout << "Simulation complete.\n";
    return 0;
}

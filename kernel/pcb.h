#ifndef _pcb_h_
#define _pcb_h_

#include "resources.h"

class Semaphore;
template<typename T>
class Future;

struct PCB {
        Shared<FileDescriptor> fd[10];
        PCB* cp[10];
        Semaphore* sp[10];
        
        Future<uint32_t>* fut;
        // true: contains an stdin/out/err
        bool std[3];

        PCB() {
            // initialize flags that represent std
            for (uint32_t i = 0; i < 3; i++) {
                std[i] = true;
                fd[i] = new FileDescriptor();
            }

            // initialize resources
            for (uint32_t i = 0; i < 10; i++) {
                if (i >= 3) {
                    fd[i] = nullptr;
                }
                cp[i] = nullptr;
                sp[i] = nullptr;
            }
        
        }


    };
    #endif
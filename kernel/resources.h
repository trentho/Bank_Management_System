#ifndef _resources_h_
#define _resources_h_

#include "ext2.h"
#include "sys.h"

    struct FileDescriptor{
        // what file am I
        Shared<Node> currentFile = Shared<Node>{};
        // where am I in the file
        uint32_t currentIndex = 0;
        // idk
        Atomic<uint32_t> ref_count{0};
        
    };


#endif
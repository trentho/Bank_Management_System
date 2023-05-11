#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "debug.h"
#include "ide.h"
#include "ext2.h"
#include "elf.h"
#include "machine.h"
#include "libk.h"
#include "config.h"
#include "atomic.h"

class Keyboard {

public:

    void readKey() {
        uint8_t scan_code = inb(0x60);
        Debug::printf("Received scan code: %x\n", scan_code);
    }

};

#endif
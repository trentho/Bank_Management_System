#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include "debug.h"
#include "ide.h"
#include "ext2.h"
#include "elf.h"
#include "machine.h"
#include "libk.h"
#include "config.h"
#include "atomic.h"

// PS/2 Controller
class Controller {

public:

    void controller_handler();

    void sending_bytes();

    int receiving_bytes();
};

#endif
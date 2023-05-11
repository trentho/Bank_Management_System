#ifndef _MOUSE_H_
#define _MOUSE_H_

#include "debug.h"
#include "ide.h"
#include "machine.h"
#include "libk.h"
#include "config.h"
#include "atomic.h"


class Mouse {
    uint8_t mouse_ticks;


    public:

    void mouse_handler();

    void mouse_wait_out();

    void mouse_wait_in();


    void mouse_write(uint8_t data);


    uint8_t mouse_read();

    void enable_mouse();



};





#endif
#include "mouse.h"

    uint8_t mouse_ticks;


    void mouse_handler(){
        mouse_ticks++;


        return;
    }


    // Wait for PS/2 controller to OK a recieve/send byte
    void mouse_wait_out(){


        for (uint32_t timeout = 0; timeout < 100000; timeout++){
            if ((inb(0x64) & 2) == 0){
                return;
            }
        }



        return;
    }

    void mouse_wait_in(){

        for (uint32_t timeout = 0; timeout < 100000; timeout++){
            if ((inb(0x64) & 1) == 1){
                return;
            }
        }



        return;

    }


    // Function that waits to send byte saying we will send a byte, then waits to send byte with command, then waits for acknowledge
    void mouse_write(uint8_t data){

        mouse_wait_out();
        outb(0x64, 0xD4);

        mouse_wait_out();
        outb(0x60, data);

        mouse_wait_in();
        
        inb(0x60);
    }

    // Function that waits to recieve a byte then recieves the byte

    uint8_t mouse_read(){
        mouse_wait_in();


        return inb(0x60);
    }


    void enable_mouse(){

        uint8_t mouse_status;
        uint8_t transform_byte_out;

        mouse_wait_out();
        outb(0x64, 0xA8);
        mouse_wait_in();
        // Enable aux mouse device (not necessarily required but recommended), receive ack from keyboard
        inb(0x60);


        mouse_wait_out();
         // Asks for compaq status of mouse
        outb(0x64, 0x20);
        mouse_wait_in();
        mouse_status = inb(0x60);
        // Gets 'compaq status' of mouse, transforms it to enable mouse
        transform_byte_out = (mouse_status | 2);

        mouse_wait_out();
        outb(0x64, 0x60);
        mouse_wait_out();
        // Tells PS/2 chip that compaq status is to be changed then send transformed byte
        outb(0x60, transform_byte_out);

        //mouse default settings
        mouse_write(0xf6);

        //mouse enabled
        mouse_write(0xf4);

        return;
    }


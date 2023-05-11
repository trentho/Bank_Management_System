#include "controller.h"

// inb (address) - read operation
// outb (address, data) - write operation
void controller_handler() {

        // flush the output buffer - read from IO Port 0x60 without testing bit 0
        inb(0x60);
        return;
}

// Poll bit 1 of the Status Register ("Input buffer empty/full") until it becomes clear, or until your time-out expires
void sending_bytes() {
    int status_register_bit1;
    Atomic<int> counter = 0;

    while (counter < 10000) {
        status_register_bit1 = inb(0x64) & 2;
        if (status_register_bit1 == 0) {
            // input buffer is empty, write data to data port
            outb(0x60, 0xF4); // enable scanning
            return;
        }
        counter.fetch_add(1);
    }
    // If the time-out expired, return an error
    Debug::panic("time-out expired");
}

// wait until bit 0 of the Status Register becomes set, then read the received byte of data from IO Port 0x60.
int receiving_bytes() {
    int status_register_bit0;
    Atomic<int> counter = 0;

    while (counter < 10000) {
        // wait until bit 0 of the Status Register becomes set
        status_register_bit0 = inb(0x64) & 1;
        if (status_register_bit0 == 1) {
            // read the received byte of data from IO Port 0x60
            return inb(0x60);
        }
        counter.fetch_add(1);
    }
    Debug::panic("bit 0 of the status register never became set");
    return -1;
}
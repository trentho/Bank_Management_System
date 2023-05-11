#include "debug.h"
#include "ide.h"
#include "ext2.h"
#include "elf.h"
#include "machine.h"
#include "libk.h"
#include "config.h"
#include "atomic.h"
#include "kernel.h"
#include "controller.h"
#include "keyboard.h"
#include "semaphore.h"

Shared<Ext2> fs = Shared<Ext2>{};

void wait() {
    while(inb(0x60) != 0xF0);
    inb(0x60); //do it again to skip key code
}

   
Shared<Node> checkFile(const char* name, Shared<Node> node) {

    return node;
}

Shared<Node> getFile(Shared<Ext2> fs, Shared<Node> node, const char* name) {
    return checkFile(name,fs->find(node,name));
}

Shared<Node> checkDir(const char* name, Shared<Node> node) {

    return node;
}

Shared<Node> getDir(Shared<Ext2> fs, Shared<Node> node, const char* name) {
    return checkDir(name,fs->find(node,name));
}

typedef volatile uint8_t init_graph[200][320];


// // example for 320x200 vga from OSDEV website
// void putpixel(int pos_x, int pos_y, char color) {
//     static init_graph *graph = (init_graph*) (void*) 0xA0000;

//     (*graph)[pos_y][pos_x] = color;
    
// }


void graphicsRectangle(uint8_t x, uint8_t y, uint32_t color) {
    init_graph *graph = (init_graph*) (void*) 0xA0000;

    for (int i = y; i < 200; i++){
        for (int j = x; j < 320; j++) {
            (*graph)[i][j] = color;
        }
    }
    
}

void drawDeposit(uint8_t x, uint8_t y, uint32_t color) {

    init_graph *deposit = (init_graph*) (void*) 0xA0000;

    for (int i = y; i < 70; i++){
        for (int j = x; j < 250; j++){
            (*deposit)[i][j] = color;
        }
    }

}

void drawWithdraw(uint8_t x, uint8_t y, uint32_t color) {

    init_graph *withdraw = (init_graph*) (void*) 0xA0000;

    for (int i = y; i < 120; i++){
        for (int j = x; j < 250; j++){
            (*withdraw)[i][j] = color;
        }
    }

}

void drawAccount(uint8_t x, uint8_t y, uint32_t color) {

    init_graph *account = (init_graph*) (void*) 0xA0000;

    for (int i = y; i < 170; i++){
        for (int j = x; j < 250; j++){
            (*account)[i][j] = color;
        }
    }

}

void drawPixel (uint8_t x, uint8_t y, uint8_t x2, uint8_t y2, uint32_t color) {

    init_graph *rectangle = (init_graph*) (void*) 0xA0000;

    for (int i = y; i < y2; i++){
        for (int j = x; j < x2; j++){
            (*rectangle)[i][j] = color;
        }
    }

}



void kernelMain(void) {
    auto d = Shared<Ide>::make(1);
    auto fs = Shared<Ext2>::make(d);
    auto root = checkDir("/",fs->root);
    auto sbin = getDir(fs,root,"sbin");
    auto init = getFile(fs,sbin,"init");

    uint32_t e = ELF::load(init);
    uint32_t userEsp = 0xefffe000 - 4*5;



    // draw the rectangle for graphics

    graphicsRectangle(0,0,0);



    // draw deposit
    drawDeposit(160/2, 30, 15);
    drawPixel(155,35,165,65,2);
    drawPixel(145,45,175,55,2);

    // draw withdraw
    drawWithdraw(160/2, 80, 15);
    drawPixel(145,95,175,105,40);

    // draw account info
    drawAccount(160/2, 130, 15);
    drawPixel(155,135,165,140,9);
    drawPixel(155,145,165,165,9);


    // Debug::printf("before graphics\n");
    // graphicsRectangle(0,0,124);

    // Debug::printf("after graphics\n");



    // Controller keyboard_controller = Controller();

    // keyboard_controller.controller_handler();


    //flush the output buffer
    inb(0x60);

    //enable the device port
    outb(0x64, 0xAE);

    outb(0x64, 0x60);
    //since we have to send two bytes, make sure the controller is ready
    while((inb(0x60)) & 1) {
        //spin
    }
    outb(0x60, 0x0);
    
    //enable scanning
    outb(0x60,  0xF4);
    
    int status_register_bit0;

    while (true) {
        // wait until bit 0 of the Status Register becomes set
        status_register_bit0 = inb(0x64) & 1;
        if (status_register_bit0 == 1) {
           
            // read the received byte of data from IO Port 0x60
            uint8_t byte_read = inb(0x60);
            while(byte_read == 0xFA) {
                byte_read = inb(0x60);
            }

            switch (byte_read) {
                case 0x0E:
                    Debug::printf("`");
                    wait();
                    continue;
                case 0x16:
                    Debug::printf("1");
                    wait();
                    continue;
                case 0x1E:
                    Debug::printf("2");
                    wait();
                    continue;
                case 0x26:
                    Debug::printf("3");
                    wait();
                    continue;
                case 0x25:
                    Debug::printf("4");
                    wait();
                    continue;
                case 0x2E:
                    Debug::printf("5");
                    wait();
                    continue;
                case 0x36:
                    Debug::printf("6");
                    wait();
                    continue;
                case 0x3D:
                    Debug::printf("7");
                    wait();
                    continue;
                case 0x3E:
                    Debug::printf("8");
                    wait();
                    continue;
                case 0x46:
                    Debug::printf("9");
                    wait();
                    continue;
                case 0x45:
                    Debug::printf("0");
                    wait();
                    continue;
                case 0x4E:
                    Debug::printf("-");
                    wait();
                    continue;
                case 0x55:
                    Debug::printf("=");
                    wait();
                    continue;
                case 0x0D:
                    Debug::printf("Tab");
                    wait();
                    continue;
                case 0x15:
                    Debug::printf("Q");
                    wait();
                    continue;
                case 0x1D:
                    Debug::printf("W");
                    wait();
                    continue;
                case 0x24:
                    Debug::printf("E");
                    wait();
                    continue;
                case 0x2D:
                    // Debug::printf("R");
                    // wait();
                    // graphicsRectangle(0,0,40);  
                    drawWithdraw(160/2, 80, 40);                  
                    continue;
                case 0x2C:
                    Debug::printf("T");
                    wait();
                    continue;
                case 0x35:
                    Debug::printf("Y");
                    wait();
                    continue;
                case 0x3C:
                    Debug::printf("U");
                    wait();
                    continue;
                case 0x43:
                    Debug::printf("I");
                    wait();
                    continue;
                case 0x44:
                    Debug::printf("O");
                    wait();
                    continue;
                case 0x4D:
                    Debug::printf("P");
                    wait();
                    continue;
                case 0x54:
                    Debug::printf("[");
                    wait();
                    continue;
                case 0x5B:
                    Debug::printf("]");
                    wait();
                    continue;
                case 0x5D:
                    Debug::printf("\\");
                    wait();
                    continue;
                case 0x58:
                    Debug::printf("Caps Lock");
                    wait();
                    continue;
                case 0x1C:
                    Debug::printf("A");
                    wait();
                    continue;
                case 0x1B:
                    Debug::printf("S");
                    wait();
                    continue;
                case 0x23:
                    Debug::printf("D");
                    wait();
                    continue;
                case 0x2B:
                    Debug::printf("F");
                    wait();
                    continue;
                case 0x34:
                    // Debug::printf("G");
                    // wait();
                    // graphicsRectangle(0,0,2);
                    drawDeposit(160/2, 30, 2);
                    continue;
                case 0x33:
                    Debug::printf("H");
                    wait();
                    continue;
                case 0x3B: 
                    Debug::printf("J");
                    wait();
                    continue;
                case 0x42:
                    Debug::printf("K");
                    wait();
                    continue;
                case 0x66:
                    // Debug::printf("Backspace");

                    //clear the colors and redraw the og shapes
                    // draw deposit
                    drawDeposit(160/2, 30, 15);
                    drawPixel(155,35,165,65,2);
                    drawPixel(145,45,175,55,2);

                    // draw withdraw
                    drawWithdraw(160/2, 80, 15);
                    drawPixel(145,95,175,105,40);

                    // draw account info
                    drawAccount(160/2, 130, 15);
                    drawPixel(155,135,165,140,9);
                    drawPixel(155,145,165,165,9);

                    wait();
                    continue;
                case 0x4B:
                    Debug::printf("L");
                    wait();
                    continue;
                case 0x4C:
                    Debug::printf(";");
                    wait();
                    continue;
                case 0x52:
                    Debug::printf("'");
                    wait();
                    continue;
                case 0x5A:
                    Debug::printf("\n");
                    wait();
                    continue;
                case 0x12:
                    Debug::printf("Left Shift");
                    wait();
                    continue;
                case 0x1A:
                    Debug::printf("Z");
                    wait();
                    continue;
                case 0x22:
                    Debug::printf("X");
                    wait();
                    continue;
                case 0x21:
                    Debug::printf("C");
                    wait();
                    continue;
                case 0x2A:
                    Debug::printf("V");
                    wait();
                    continue;
                case 0x32:
                    // Debug::printf("B");
                    // wait();
                    // graphicsRectangle(0,0,9);
                    drawAccount(160/2, 130, 9);
                    continue;
                case 0x31:
                    Debug::printf("N");
                    wait();
                    continue;
                case 0x3A:
                    Debug::printf("M");
                    wait();
                    continue;
                case 0x41:
                    Debug::printf(",");
                    wait();
                    continue;
                case 0x49:
                    Debug::printf(".");
                    wait();
                    continue;
                case 0x4A:
                    Debug::printf("/");
                    wait();
                    continue;
                case 0x59:
                    Debug::printf("Right Shift");
                    wait();
                    continue;
                case 0x14:
                    Debug::printf("Control");
                    wait();
                    continue;
                case 0x1F:
                    Debug::printf("Left Windows");
                    wait();
                    continue;
                case 0x11:
                    Debug::printf("Alt");
                    wait();
                    continue;
                case 0x29:
                    Debug::printf(" ");
                    wait();
                    continue;
                case 0x27:
                    Debug::printf("Right Windows");
                    wait();
                    continue;
                case 0x2F:
                    Debug::printf("Menus");
                    wait();
                    continue;
                case 0x70:
                    Debug::printf("Insert");
                    wait();
                    continue;
                case 0x6C:
                    Debug::printf("Home");
                    wait();
                    continue;
                case 0x7D:
                    Debug::printf("Page Up");
                    wait();
                    continue;
                case 0x71:
                    Debug::printf("Delete");
                    wait();
                    continue;
                case 0x69:
                    Debug::printf("End");
                    wait();
                    continue;
                case 0x7A:
                    Debug::printf("Page Down");
                    wait();
                    continue;
                case 0x75:
                    Debug::printf("Up Arrow");
                    wait();
                    continue;
                case 0x6B:
                    Debug::printf("Left Arrow");
                    wait();
                    continue;
                case 0x72:wait();
                    Debug::printf("Down Arrow");
                    wait();
                    continue;
                case 0x74:
                    Debug::printf("Right Arrow");
                    wait();
                    continue;
                case 0x77:
                    Debug::printf("Num Lock");
                    wait();
                    continue;
                case 0x7C:
                    Debug::printf("*");
                    wait();
                    continue;
                // default:
                //     Debug::printf("Unknown Character Code: %x\n", byte_read);
                //     continue;
            }
            

        }
    }


    char** argv = new char*[1];
    argv[0] = (char*)"init";
    memcpy((void*) (userEsp + 4), &argv, 4);
    // Current state:
    //     - %eip points somewhere in the middle of kernelMain
    //     - %cs contains kernelCS (CPL = 0)
    //     - %esp points in the middle of the thread's kernel stack
    //     - %ss contains kernelSS
    //     - %eflags has IF=1
    // Desired state:
    //     - %eip points at e
    //     - %cs contains userCS (CPL = 3)
    //     - %eflags continues to have IF=1
    //     - %esp points to the bottom of the user stack
    //     - %ss contain userSS
    // User mode will never "return" from switchToUser. It will
    // enter the kernel through interrupts, exceptions, and system
    // calls.
    switchToUser(e,userEsp,0);
    Debug::panic("implement switchToUser in machine.S\n");
}
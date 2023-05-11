#include "sys.h"
#include "stdint.h"
#include "idt.h"
#include "debug.h"
#include "machine.h"
#include "semaphore.h"
#include "ext2.h"
#include "resources.h"
#include "pcb.h"
#include "kernel.h"
#include "future.h"
#include "vmm.h"
#include "physmem.h"
#include "elf.h"
#include "libk.h"



//extern Shared<Ext2> fs;




// typedef volatile uint8_t init_graph[200][320];


// // example for 320x200 vga from OSDEV website
// void putpixel(int pos_x, int pos_y, char color) {
//     static init_graph *graph = (init_graph*) (void*) 0xA0000;

//     (*graph)[pos_y][pos_x] = color;
    
// }






extern "C" int sysHandler(uint32_t eax, uint32_t *frame) {

    using namespace gheith;
    using namespace VMM;
    using namespace PhysMem;

    PCB* this_pcb = current()->r;

    auto u_eip = (uint32_t*) frame[0];
    auto u_esp = (uint32_t*) frame[3];

    // get values from user esp
    uint32_t fd = u_esp[1];
    char* buff = (char*) u_esp[2];
    uint32_t nbytes = u_esp[3];
    size_t i = 0;
    //uint32_t std_index = 0;
    // Debug::printf("eax: %d\n", eax);
    // Debug::printf("fd: %d\n", fd);
    // syscalls
    switch (eax) {
        
    case 0: /* exit */
    // if child, tell parent, give status to parent
    // stop its work
    //deallocate resources (unless shared)
    // deallocate used frames (unless shared)
    // if a parent process calls wait on this process, the status should be set to rc
        Debug::printf("in exit\n");
        //if (this_pcb->cp[fd] != nullptr) {
            Debug::printf("setting: \n", fd);
            this_pcb->cp[fd]->fut->set(fd);
        //}
        stop();
        break;
    
    case 1: { /* write */
        //Debug::printf("in write\n");
        // makes sure file descriptor can go out to console & buff isn't trying to access something it shouldn't
        if (fd > 2 || (buff >= (char*)0xfec00000 && buff <= (char*)0xfec00000 + 4096) || (
                buff >= (char*)0xfee00000 && buff <= (char*)0xfee00000 + 4096) || buff < (char*)0x80000000) {
            return -1;
        }

        // if nbytes + buff overlaps into the ioapic or localapic...
        if ((uint32_t) buff + nbytes > 0xfec00000 && (uint32_t) buff + nbytes < 0xfec00000 + 4096) {
            nbytes = 0xfec00000 - (uint32_t) buff;
        } else if ((uint32_t) buff + nbytes > 0xfee00000 && (uint32_t) buff + nbytes < 0xfee00000 + 4096) {
            nbytes = 0xfee00000 - (uint32_t) buff;
        }

        while (i < nbytes) {
            Debug::printf("%c", buff[i]);
            i++;
        }
        return nbytes;
    }
    
    
    // 0 => child, +ve => parent, -ve => error
    case 2: { /* fork */ 
        Debug::printf("in fork\n");

        // check how many child processes exist
        while (this_pcb->cp[i] != nullptr) {
            if (i >=10) {
                return -1;
            }
            i++;
        }

        // new thread made
        auto child = child_thread([u_esp, u_eip] {

            Debug::printf("In child thread, about to switch to user\n");
            
            switchToUser((uint32_t) u_eip, (uint32_t) u_esp, 0);
            Debug::printf("switched\n");
        });
        


        Debug::printf("Copying files & semaphores\n");
        // copy files and semaphores
        for (uint32_t ind = 0; ind < 10; ind++) {
		    if (this_pcb->sp[i] != nullptr) {
			    child->r->sp[i] = this_pcb->sp[i];
		    }
            child->r->fd[i] = this_pcb->fd[i];
	    }

        //child->saveArea.cr3 = current()->saveArea.cr3;

        // deep copy vm
        // pd can hold 4096 / 4 entries, & half are shared, so we start at 4096/4/2 (or 512)
        // pd[pdi] & 1  to get present bit
        for (uint32_t pdi = 512; pdi < 4096/4; pdi++) {
		    auto p_pde = current()->pd[pdi];
		    if ((p_pde & 1) == 1) {
                //map(current()->pd, p_pde, );
		        auto p_pt = (uint32_t*) (p_pde & 0xFFFFF000);
		        auto c_pde = child->pd[pdi];
		        auto c_pt = (uint32_t*) (c_pde & 0xFFFFF000);
		
		        if ((c_pde & 1) == 0) {
			        c_pt = (uint32_t*) PhysMem::alloc_frame();
                    // map(c_pt,)
			        child->pd[pdi] = uint32_t(c_pt) | 7;
		        }
                // now looping through the page table entries
		        for (uint32_t pti = 0; pti < 4096/4; pti++) {
			        auto c_pte = c_pt[pti];
			        if ((c_pte & 1) == 0) {
			            auto p_pte = p_pt[pti];
			            if ((p_pte & 1) == 1) {
			                auto p_frame = p_pte & 0xFFFFF000;
			                auto c_frame = PhysMem::alloc_frame();
			                memcpy((void*)c_frame,(void*)p_frame,PhysMem::FRAME_SIZE);
			                c_pt[pti] = uint32_t(c_frame) | 7;
                        }
                    }
                }
		    }
        }
        
            
        this_pcb->cp[i] = child->r;
        readyQ.add(child);
        Debug::printf("i: %d\n", i);
    // copy files, sem, deep copy private vm
    // ignore children
    // create a new tcb
    // make thread-like function that returns pointer to tcb
    // child thread needs to enter switch to user(pc, esp, eax)
    // here, eax in child process should be 0
    // esp point to same as esp parent 
    // make a pde, pt, pp, memcopy all 4096 bytes of however many pages the parent has
        return i + 10;
    }

    case 3: { /* sem */
    // takes initial
//     there are already 10 semaphores
// return an implementation-defined semaphore descriptor
        Debug::printf("in sem\n");
        while (this_pcb->sp[i] == nullptr) {
            i++;
            if (i == 10) {
                return -1;
            }
        }
        this_pcb->sp[i] = new Semaphore(fd);

        return i + 20;
    }

    case 4: { /* up */
    // Debug::printf("in up, fd: %d\n", fd);
        // takes id
        if (fd >= 20) {
            fd -= 20;
        }
        if (this_pcb->sp[fd] == nullptr) {
            return -1;
        }
        this_pcb->sp[fd]->up();
        return 0;
        
    }

    case 5: { /* down */
        Debug::printf("in down, fd: %d\n", fd);
        // takes id
        if (fd >= 20) {
            fd -= 20;
        };
        if (this_pcb->sp[fd] == nullptr) {
            return -1;
        }
        this_pcb->sp[fd]->down();
        return 0;

    }

    case 6: { /* close */
    Debug::printf("in close\n");

        if (fd < 0 || fd > 29) {
            return -1;
        }

        if (fd <= 2) {
            if (current()->r->std[fd]) {
                current()->r->std[fd] = false;
            }
        }

        Debug::printf("closing %d\n", fd);

        current()->r->fd[fd] = Shared<FileDescriptor>{};
        return 0;

    }

    case 7: { /* shutdown */
    Debug::printf("in shutdown\n");
        Debug::shutdown();
    }

    case 8: { /* wait */
        
        Debug::printf("in wait, *buff: %d, fd: %d\n", *buff, fd);
        
        //Debug::printf("this_pcb->cp[fd - 10] == nullptr: %d\nthis_pcb->fd[fd - 10] == nullptr: %d\n", this_pcb->cp[fd - 10] == nullptr, this_pcb->fd[fd - 10] == nullptr);
        if (this_pcb->cp[fd - 10] == nullptr || (uint32_t) buff < 0x80000000 || fd < 10 || fd > 19) {
            Debug::printf("in if statement\n");
            return -1;
        }

        //Debug::printf("after if statement\n");
        //Debug::printf("buff: %x, *buff: %d, &buff: %x\n", buff, *buff, &buff);

        this_pcb->cp[fd-10]->fut = new Future<uint32_t>;
        //Debug::printf("made a fut\n");
        //ASSERT(this_pcb->fut != nullptr);
        ASSERT(this_pcb->cp[fd-10]->fut != nullptr);
        uint32_t temp = this_pcb->cp[fd-10]->fut->get();
        memcpy(&buff, &temp, 4);
        
        Debug::printf("returning 0\n");
        return 0;
        
//         return negative if:
// id is an invalid child pid
// status is not in user-space
// return 0 with the exit code (rc) of the child process written to status
    // Debug::printf("(uint32_t*)buff: %x, *(uint32_t*)buff: %x\n", (uint32_t*)buff, *(uint32_t*)buff);
    //*(uint32_t*)buff =  this_pcb->fut->get();

    }

    // returning indicates an error 
    // arg0 is the name of the program by convention
    // a nullptr indicates end of arguments

    case 9: { /* execl */
        Debug::printf("in execl\n");
        // u_esp [1]
        char* path = (char*) fd;
        // u_esp[2]
        // pointer to args
        auto argv = (char**) &u_esp[2];

        Shared<Node> node = fs->find(fs->root, path);
        
        // get argc
        i = 2;
        char* temp = new char;
        while ((uint32_t*) u_esp[i] != nullptr) {
            i++;
        }
        uint32_t argc = i - 2;
        uint32_t data_len = 0;
        uint32_t total_size = 8;

        // get total length of data
        for (uint32_t i = 2; i < argc + 2 ; i++) {
            temp = (char*) u_esp[i];
            uint32_t strlen = 0;
            while (temp[strlen] != 0) {
                strlen++;
            }
            data_len += strlen + 1;
        }

        
        // save data into buffer
        auto buffer = new char* [argc + 1];
        i = 0;
        for (uint32_t arg = 2; arg < argc + 2 ; arg++) {
            temp = (char*) u_esp[arg];
            uint32_t strlen = 0;
            while (temp[strlen] != 0) {
                strlen++;
            }
            buffer[i] = (char*) malloc(strlen + 1);
	        memcpy(buffer[i], argv[i], strlen + 1);
            uint32_t padding = strlen % 4;
            total_size += (uint32_t) strlen + (4 - padding);
            Debug::printf("totalsize: %d\n", total_size);
            i++;
        }

        buffer[argc] = 0;

        for (uint32_t x = 0; x < argc + 1; x++) {
            Debug::printf("***********buffer[x]: %s\n", buffer[x]);
        }

        // args
        total_size += argc * 4;
        // null terminator
        total_size += 4;

        Debug::printf("totalsize: %d\n", total_size);

        
        //Debug::printf("&argc: %x, argc: %d, argv: %x\n", &argc, argc, argv);
        // Debug::printf("total strlen: %d, &buffer[0]: %x, &buffer[1]: %x\n", total_strlen, &buffer[0], &buffer[1]);
        

        if (node == nullptr) {
            return -1;
        }

        // deleting vm
        for (uint32_t pdi = 512; pdi < 4096/4; pdi++) {
		    auto p_pde = current()->pd[pdi];
		    if ((p_pde & 1) == 1) {
                //map(current()->pd, p_pde, );
		        auto p_pt = (uint32_t*) (p_pde & 0xFFFFF000);
                // now looping through the page table entries
		        for (uint32_t pti = 0; pti < 4096/4; pti++) {
			        auto p_pte = p_pt[pti];
			        if ((p_pte & 1) == 1) {
                        Debug::printf("va: %x\n", pdi << 22 | pti << 12);
                        uint32_t va = pdi << 22 | pti << 12;
                        if (!(va >= kConfig.ioAPIC && va < kConfig.ioAPIC + 4096) && !(va >= kConfig.localAPIC && va <= kConfig.localAPIC + 4096)) {
                            Debug::printf("**va: %x\n", pdi << 22 | pti << 12);
                            unmap(current()->pd, va);
			                // auto p_frame = p_pte & 0xFFFFF000;
			                // dealloc_frame(p_frame);
                            // invlpg(pdi << 22 | pti << 12);
                        }
                    }
                    
                }
		    }
        }

        Debug::printf("deleted vm\n");

        for (uint32_t i = 0; i < 10; i++) {
            current()->r->cp[i] = nullptr;
            current()->r->sp[i] = nullptr;
        }

        // Debug::printf("%x - %d: %x\n", 0xefffe000, total_size, 0xefffe000 - total_size);
        

        auto new_esp = (uint32_t*) (0xefffe000 - total_size);
        Debug::printf("esp: %x, totalsize: %d, esp + totalsize: %x\n", new_esp, total_size, new_esp + sizeof(i) * 3);
        // Debug::printf("0xefffe000 - new_esp: %d\n", 0xefffe000 - (uint32_t) new_esp);



        // push the new arguments
        new_esp[0] = argc;
        new_esp[1] = (uint32_t)&new_esp[2];


        //argc & argv + num args + NULL
        
        // uint32_t data_offset = 0;
        // uint32_t l = 7;
        // uint32_t naddress = (uint32_t) 0xefffe000; 
        // Debug::printf("naddress: %x\n",naddress);
        // for (uint32_t i = argc; i > 0; i--) {
        //     uint32_t len = K::strlen(buffer[i - 1]);
        //     uint32_t padding = 4 - (len % 4);
        //     data_offset = (len + padding) / 4;
        //     Debug::printf("data_offset: %d\n", data_offset);
        //     naddress -= sizeof(1)*data_offset;
        //     // Debug::printf("naddress: %x\n",naddress);

            
        //     // Debug::printf("&new_esp[7]: %x\n", &new_esp[7]);
        //     // Debug::printf("len: %d, padding: %d, buffer[%d]: %s, naddress - sizeof(1)*%d: %x\n", len, padding, i - 1, buffer[i - 1], data_offset, naddress);
        //     // memcpy((void*)(naddress), &buffer[i - 1], len + 1);
        //     Debug::printf("naddress: %x\n", naddress);
        //     // Debug::printf("%s\n", tempp);
        //     // data_offset = (len + padding) / 4;
        //     // Debug::printf("Offset: %d\n, data_offset");
        //     Debug::printf("new_esp[%d]: %s\n", l, new_esp[l]);
        //     l++;
            
        // }



        uint32_t x = 0;
        i = 2;
        data_len = 0;
        uint32_t address = 2 + argc + 1;
        while (i < argc + 2) {
            // Debug::printf("argc: %d\n", argc);
            // Debug::printf("address: %d\n", address);
            // Debug::printf("buffer[%d]: %s\n", x, buffer[x]);
            uint32_t len = K::strlen(buffer[x]);
            uint32_t padding = 4 - (len % 4);
            // uint32_t offset = (len + padding) / 4;
            new_esp[i] = (uint32_t)&new_esp[address];
            Debug::printf("i set the new_esp[i], will now memcpy\n");
            //Debug::printf("buffer[%d]: %s\n",x, buffer[x]);
            //memcpy((void*)&new_esp[i], buffer[x], len + 1);
            Debug::printf("new_esp[%d]: %x\n", i, &new_esp[address]);
            // Debug::printf("new_esp[%d]: %s\n", address, *(uint32_t*)new_esp[address]);
            // address += offset;
            data_len += len + padding;
            i++;
            x++;
        }

        Debug::printf("data_len: %d\n", data_len);
        new_esp[address - 1] = 0;

        Debug::printf("&new_esp[address]: %x, buffer[0]: %s\n", &new_esp[address], buffer[0]);

        memcpy((char*)&new_esp[address], buffer, data_len);

        Debug::printf("new_esp[0]: %d\n", new_esp[0]);
        Debug::printf("new_esp[1]: %x\n", new_esp[1]);

        for (uint32_t i = 2; i < 11; i++) {
            if (i < 7) {
                Debug::printf("new_esp[%d]: %x\n", i, new_esp[i]);
            } else {
                Debug::printf("new_esp[%d]: %s, &new_esp[i]: %x\n", i, new_esp[i], &new_esp[i]);  
                 
            }
        }

        Debug::printf("path: %s\n", path);

        new_esp[1] = (uint32_t)&new_esp[2];
        // Debug::printf("new_esp[1]: %x\n", new_esp[1]);
        // for (uint32_t x = 0; x < 14; x++) {
        //     Debug::printf("new_esp[%d]: %x\n", x,new_esp[x]);
        // }
    //Debug::printf("new_esp[1]: %x, [2]: %x\n", u_esp[1], u_esp[2]);
    // argv[0]
    // memcpy();
        uint32_t e = ELF::load(node);

        ASSERT(node != nullptr);
    
    // // bad elf files
    //     node == nullptr;
        Debug::printf("switching to user\n");
        switchToUser(e, (uint32_t)new_esp, 0);
        Debug::panic("*** implement switchToUser");
        return -1;
    }

    case 10: { /* open */
    Debug::printf("in open\n");
        // Debug::printf("fn: %s\n", (char*) fd);
        // Debug::printf("opening a file, fd: %d\n", fd);

        Shared<Node> file = fs->find(fs->root, (char*) fd);

        if (file == Shared<Node> {}) {
            Debug::printf("file is null\n");
            return -1;
        }        
        
        while (gheith::current()->r->fd[i] != nullptr) {
            Debug::printf("in while loop. i = %d\n", i);
            i++;
            if (i >= 10) {
                return -1;
            }
        }

        current()->r->fd[i] = new FileDescriptor();
        current()->r->fd[i]->currentFile = file;

        Debug::printf("returning, i = %d\n", i);
        
        return i;
        break;
    }


    case 11: { /* len */
    Debug::printf("in len, fd: %d\n", fd);

        if (fd < 0 || fd > 9 || current()->r->fd[fd] == Shared<FileDescriptor>{}) {
            return -1;
        }

        // if fd points to stdin/out/err
        if (current()->r->std[fd]) {
            return 0;
        }


        Shared<Node> file = gheith::current()->r->fd[fd]->currentFile;

	    // bad fd
	    if (file == nullptr){
		    return -1;
	    }

        Debug::printf("file is not null, will now try to return size_in bytes\n");
	        return (ssize_t) file->size_in_bytes();
        }

    case 12: { /* read */
    Debug::printf("in read, fd: %x, buff: %x\n", fd, buff);
        // makes sure file descriptor can be read & buff isn't trying to access something it shouldn't
        if (fd < 0 || fd > 9 || (buff >= (char*)0xfec00000 && buff <= (char*)0xfec00000 + 4096) || (
                buff >= (char*)0xfee00000 && buff <= (char*)0xfee00000 + 4096) || buff < (char*)0x80000000) {
            return -1;
        }

        if (current()->r->fd[fd] == Shared<FileDescriptor>{}) {
            return -1;
        }

        // IF FD POINTS TO STDIN/OUT/ERR
        if (current()->r->std[fd]) {
            return -1;
        }

        

        // if nbytes + buff overlaps into the ioapic or localapic... we hit the end of the file...
        if ((uint32_t) buff + nbytes > 0xfec00000 && (uint32_t) buff + nbytes < 0xfec00000 + 4096) {
            nbytes = 0xfec00000 - (uint32_t) buff;
        } else if ((uint32_t) buff + nbytes > 0xfee00000 && (uint32_t) buff + nbytes < 0xfee00000 + 4096) {
            nbytes = 0xfee00000 - (uint32_t) buff;
        }
        
        // if we hit the end of the file...
        if (current()->r->fd[fd]->currentIndex + nbytes > current()->r->fd[fd]->currentFile->size_in_bytes()) {
            nbytes = current()->r->fd[fd]->currentFile->size_in_bytes() - current()->r->fd[fd]->currentIndex;
        }

        current()->r->fd[fd]->currentFile->read_all(current()->r->fd[fd]->currentIndex, nbytes, buff);

        current()->r->fd[fd]->currentIndex += nbytes;
        // Debug::printf("current index: %d\n", 
        // current()->r->fd[fd]->currentIndex );

        return nbytes;


    // update offset whenever you read to where you ended off in the file
    }

    case 13: { /* seek */
    Debug::printf("in seek, fd: %d\n", fd);
        if (current()->r->fd[fd] == Shared<FileDescriptor>{} || fd < 0 || fd > 9 || (current()->r->std[fd])) {
            return -1;
        }

        current()->r->fd[fd]->currentIndex = (uint32_t) buff;
        
        return current()->r->fd[fd]->currentIndex;

    }

    
    }

    //MISSING();
    return 0;
}

void SYS::init(void) {
    IDT::trap(48,(uint32_t)sysHandler_,3);
}
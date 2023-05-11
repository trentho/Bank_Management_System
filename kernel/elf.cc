#include "elf.h"
#include "debug.h"
#include "ext2.h"

uint32_t ELF::load(Shared<Node> file) {
    ElfHeader* elf_header = new ElfHeader();
    file->read_all(0, sizeof(ElfHeader), (char *) elf_header);

    if (elf_header->magic0 != 0x7f) {Debug::panic("magic0 is wrong");}
    if (elf_header->magic1 != 'E') {Debug::panic("magic1 is wrong");}
    if (elf_header->magic2 != 'L') {Debug::panic("magic2 is wrong");}
    if (elf_header->magic3 != 'F') {Debug::panic("magic3 is wrong");}

    auto ph_offset = elf_header->phoff;
    for (unsigned i = 0; i < elf_header->phnum; i++) {
        ProgramHeader* pg_header = new ProgramHeader();
        file->read_all(ph_offset, sizeof(ProgramHeader), (char *) pg_header);

        file->read_all(pg_header->offset, pg_header->filesz, (char *) pg_header->vaddr);
        ph_offset += elf_header->phentsize;
    }

    return elf_header->entry;
}


















































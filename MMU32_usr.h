//Memory Mapping Utility (AARCH32)

#ifndef MMU32_USR_H
#define MMU32_USR_H

#include <stdbool.h>
#include <stdint.h>

//Returns true if "mmu_init()" has already been called.
bool mmu_is_active(void);
//Initializes MMU procedure.
//This function must be called before calling any other functions in this header.
//Returns true if initialization is successful.
bool mmu_init(void);

uint32_t mmu_get_phys_from_virt(void *virtaddr);
void *mmu_get_virt_from_phys(uint32_t physaddr);

#endif

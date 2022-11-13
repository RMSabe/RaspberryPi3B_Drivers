//Memory Mapping Utility (AARCH32)

#include "MMU32_usr.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

/*
"MMU_WAIT_KERNEL_RESPONSE"
If defined, application will call kernel and wait for response before proceeding.
Else, application will wait a specific time (defined in MMU_WAIT_TIME_US) is microseconds before proceeding.
*/
#define MMU_WAIT_KERNEL_RESPONSE

#define MMU_PROC_FILE_DIR "/proc/MMU32"
#define MMU_WAIT_TIME_US 1

#define MMU_DATAIO_SIZE_BYTES 5

#define MMU_CMD_GET_PHYSICAL_ADDR 1
#define MMU_CMD_GET_VIRTUAL_ADDR 2

#define MMU_CMD_KERNEL_RESPONSE 0xFF

int mmu_proc_fd = -1;
void *mmu_data_io = NULL;

void mmu_wait(void)
{
	clock_t start_time = clock();
	while(clock() < (start_time + MMU_WAIT_TIME_US));
	return;
}

bool mmu_is_active(void)
{
	return (mmu_proc_fd >= 0);
}

bool mmu_init(void)
{
	if(mmu_is_active()) return true;

	mmu_proc_fd = open(MMU_PROC_FILE_DIR, O_RDWR);
	if(mmu_proc_fd < 0) return false;

	mmu_data_io = malloc(MMU_DATAIO_SIZE_BYTES);
	return true;
}

#ifdef MMU_WAIT_KERNEL_RESPONSE
void mmu_call_kernel(void)
{
	uint8_t *pbyte = (uint8_t*) mmu_data_io;
	write(mmu_proc_fd, mmu_data_io, MMU_DATAIO_SIZE_BYTES);

	do{
		read(mmu_proc_fd, mmu_data_io, MMU_DATAIO_SIZE_BYTES);
	}while(pbyte[0] != MMU_CMD_KERNEL_RESPONSE);

	return;
}
#else
void mmu_call_kernel(void)
{
	write(mmu_proc_fd, mmu_data_io, MMU_DATAIO_SIZE_BYTES);
	mmu_wait();
	read(mmu_proc_fd, mmu_data_io, MMU_DATAIO_SIZE_BYTES);
	return;
}
#endif

uint32_t mmu_get_phys_from_virt(void *virtaddr)
{
	uint8_t *pbyte = (uint8_t*) mmu_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[1];
	pbyte[0] = MMU_CMD_GET_PHYSICAL_ADDR;
	puint[0] = (uint32_t) virtaddr;

	mmu_call_kernel();
	return puint[0];
}

void *mmu_get_virt_from_phys(uint32_t physaddr)
{
	uint8_t *pbyte = (uint8_t*) mmu_data_io;
	uint32_t *puint = (uint32_t*) &pbyte[1];
	pbyte[0] = MMU_CMD_GET_VIRTUAL_ADDR;
	puint[0] = physaddr;

	mmu_call_kernel();
	return (void*) puint[0];
}

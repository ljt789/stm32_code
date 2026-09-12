#ifndef  __BOOTLOADER_H
#define  __BOOTLOADER_H

#include <stdint.h>
#include <stdio.h>
#define stm32_flash_baseaddr 0x08000000     /*内部flash的起始地址*/
#define stm32_flash_size     0x00100000U    /*内部flash的大小*/

#define stm32_rom_baseaddr 0x20000000
#define stm32_rom_size     0x20000

#define bootloader_failed  -1
#define bootloader_success 1

__asm void set_up_sp_pc(uint32_t addr_sp,uint32_t addr_reset);
uint8_t jump_app(uint32_t addr);

#endif
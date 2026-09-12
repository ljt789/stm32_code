#ifndef __FLASH_H
#define __FLASH_H
#include <stdint.h>
#define STM32_FLASH_BASE 0x08000000
#define STM32_FLASH_SIZE 0x100000 /* STM32 FLASH SIZE*/
#define STM32_FLASH_WAITTIME 50000
#define flash_read_error 1
#define flash_read_success 0


#define flash_write_error 1
#define flash_write_success 0

#define flash_write_angle 1
/* FLASH BASE ADDR*/
/*SECTOR 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_0 ((uint32_t )0x08000000)
/*SECTOR 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1 ((uint32_t )0x08004000)
/*SECTOR 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2 ((uint32_t )0x08008000)
/*SECTOR 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3 ((uint32_t )0x0800C000)
/*SECTOR 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_4 ((uint32_t )0x08010000)
/*SECTOR 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_5 ((uint32_t )0x08020000)
/*SECTOR 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6 ((uint32_t )0x08040000)
/*SECTOR 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7 ((uint32_t )0x08060000)
/*SECTOR 8, 128 Kbytes */
#define ADDR_FLASH_SECTOR_8 ((uint32_t )0x08080000)
/*SECTOR 9, 128 Kbytes */
#define ADDR_FLASH_SECTOR_9 ((uint32_t )0x080A0000)
/*SECTOR 10,128 Kbytes */
#define ADDR_FLASH_SECTOR_10 ((uint32_t )0x080C0000)
/*SECTOR 11,128 Kbytes */
#define ADDR_FLASH_SECTOR_11 ((uint32_t )0x080E0000)

uint8_t flash_erase_sector(uint32_t sector);
uint8_t flash_find_sector(uint32_t addr);
uint8_t stmflash_write(uint32_t addr, const void *buf, uint32_t len);
uint8_t stmflash_read (uint32_t addr, void *buf, uint32_t len);



#endif
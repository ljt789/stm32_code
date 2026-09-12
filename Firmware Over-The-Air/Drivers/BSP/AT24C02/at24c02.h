
#ifndef __AT24C02_H
#define __AT24C02_H
#include "stm32f4xx_hal.h"
#define AT24C02_ADDR_WRITE  0xA0
#define AT24C02_ADDR_READ   0xA1

void AT24C02_BSP_INIT(void);
void Ping_AT24C02(void);
uint8_t Write_byte_at24c02(uint8_t addr,uint8_t data);

uint8_t Read_byte_at24c02(uint8_t addr);
#endif

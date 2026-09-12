#include <stmflash.h>
#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>

uint8_t flash_erase_sector(uint32_t sector);
uint8_t flash_find_sector(uint32_t addr);
uint8_t stmflash_write(uint32_t addr, const void *buf, uint32_t len);
uint8_t stmflash_read (uint32_t addr, void *buf, uint32_t len);
/**
* @brief 从stm32片内flash读取数据
* @param addr:读取的起始地址
* @param buf :接收缓冲区
* @param len :读取长度
* @retval      flash_read_success : 读取成功
* @retval      flash_read_error   : buf 为空, 或 addr 越界
*/
uint8_t stmflash_read (uint32_t addr, void *buf, uint32_t len){
//0.判断buf是否为空
if(buf==NULL) return flash_read_error;
//1.判断地址是否小于启始地址
if(addr<STM32_FLASH_BASE) return flash_read_error;
//2.判断地址加长度是否大于终止地址
if(addr+len>STM32_FLASH_BASE+STM32_FLASH_SIZE) return flash_read_error;
//3.拷贝数据
memcpy(buf,(uint32_t *)addr,len);
return flash_read_success;
}

/**
* @brief 向stm32片内flash写入数据
* @param addr:写入   的起始地址
* @param buf :接收缓冲区
* @param len :读取长度
* @retval      flash_read_success : 读取成功
* @retval      flash_read_error   : buf 为空, 或 addr 越界
* @note        写只能是将1变0，不能将0变1,所以写之前必须擦除，擦除之后均为FF
* @note        擦除的最小单元是扇区，对于stm32f4,扇区的大小不是统一的
* @note        对于当前程序，设定只能
*/

uint8_t stmflash_write(uint32_t addr, const void *buf, uint32_t len){

uint8_t sector_begin_num;
uint8_t sector_end_num;
uint8_t status;
const uint8_t *p_data = (const uint8_t *)buf;
//s1.参数校验

if(buf==NULL) return flash_write_error;
if(len==0) return flash_write_error;
//1.判断地址是否小于启始地址
if(addr<STM32_FLASH_BASE) return flash_write_error;
//2.判断地址加长度是否大于终止地址
if(addr+len>STM32_FLASH_BASE+STM32_FLASH_SIZE) return flash_write_error;

//2.获取扇区地址
sector_begin_num=flash_find_sector(addr);
sector_end_num=flash_find_sector(addr+len-1);

//3.擦除扇区
for(uint8_t i=sector_begin_num;i<=sector_end_num;i++){
    status=flash_erase_sector(i);
    if(status!=HAL_OK) 
        return flash_write_error;
    }

//4.解锁
HAL_FLASH_Unlock();
//5.写入对应的flash地址中

for(uint32_t i=0;i<len;i++)
{
if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, addr+i, p_data[i])!=HAL_OK)   
    {
            HAL_FLASH_Lock();      
            return flash_write_error;
    }

}

//6.上锁
HAL_FLASH_Lock();

return flash_write_success;
}


/**
* @brief  输入写入地址，返回flash中的扇区地址
* @param  addr:         起始地址
* @retval sector_addr : 扇区地址
*/
uint8_t flash_find_sector(uint32_t addr){
uint8_t sector_addr;

if((addr>=STM32_FLASH_BASE)&&(addr<STM32_FLASH_BASE+4*0x4000)){//属于0 1 2 3
    sector_addr=(addr-STM32_FLASH_BASE)/(0x4000);
}
else if(addr>=ADDR_FLASH_SECTOR_4&&addr<ADDR_FLASH_SECTOR_5){
    sector_addr=4;
}
else if(addr>=ADDR_FLASH_SECTOR_5&&addr<ADDR_FLASH_SECTOR_5+7*0x20000){
    sector_addr=5+((addr-ADDR_FLASH_SECTOR_5)/(0x20000));
}
else{
    return -1;
}
return sector_addr;
}

/**
* @brief  擦除扇区
* @param  addr:         起始地址
* @retval sector_addr : 片内地址
*/
uint8_t flash_erase_sector(uint32_t sector){
    FLASH_EraseInitTypeDef pEraseInit;
		uint32_t  SectorErro;
		uint8_t status;
	
	pEraseInit.Banks=FLASH_BANK_1;
	pEraseInit.NbSectors=1;
	pEraseInit.Sector=sector;
	pEraseInit.TypeErase=FLASH_TYPEERASE_SECTORS;
	pEraseInit.VoltageRange=FLASH_VOLTAGE_RANGE_3;
	
    //解锁flash
    HAL_FLASH_Unlock();
    
    //擦除扇区
    status = HAL_FLASHEx_Erase(&pEraseInit, &SectorErro);

    //上锁flash
    HAL_FLASH_Lock();
    return status;

}

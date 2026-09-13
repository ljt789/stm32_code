#include <bootloader.h>
#include <stdint.h>



/**
*@brief 跳转到APP区
*@param addr:app区的flash入口地址
*/
uint8_t jump_app(uint32_t addr){
uint32_t app_sp_addr;
uint32_t app_reset_addr;

//1.判断地址是否合法
if(addr<stm32_flash_baseaddr||addr>stm32_flash_baseaddr+stm32_flash_size){
    return bootloader_failed;
}

app_sp_addr=*((uint32_t *)addr);       //栈地址
app_reset_addr=*((uint32_t *)(addr+4));//reset复位地址

//2.判断栈是否在RAM中
if(app_sp_addr<stm32_rom_baseaddr||app_sp_addr>stm32_rom_baseaddr+stm32_rom_size){
    return bootloader_failed;
}

set_up_sp_pc(app_sp_addr,app_reset_addr);
    return bootloader_success;
}

/**
*@brief 设置栈地址以及设置pc到app区
*@param addr_sp:app区的栈地址
*@param addr_reset :app区的reset向量表地址
*/
__asm void set_up_sp_pc(uint32_t addr_sp,uint32_t addr_reset){

    //1.设置sp栈指针
    MSR MSP, r0
    //2.跳转app 设置pc指针
    BX r1
}
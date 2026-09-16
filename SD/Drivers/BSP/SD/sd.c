#include <sd.h>
#include "stm32f4xx_hal.h"
/**
*@brief SD卡的硬件初始化
*@param SD_HandleTypeDef *hsd:
*/
void HAL_SD_MspInit(SD_HandleTypeDef *hsd){
GPIO_InitTypeDef gpio_init_struct;

/*初始化时钟*/
SDI0_CLK_ENABLE() ;

SDI0_D0_CLK_ENABLE();
SDI0_D1_CLK_ENABLE();      
SDI0_D2_CLK_ENABLE();    
SDI0_D3_CLK_ENABLE();  
SDI0_SCK_CLK_ENABLE();        
SDI0_CMD_CLK_ENABLE();     

gpio_init_struct.Alternate=GPIO_AF12_SDIO ;
gpio_init_struct.Mode=GPIO_MODE_AF_PP;
gpio_init_struct.Pin=SDI0_D0_PIN;
gpio_init_struct.Pull=GPIO_PULLUP;
gpio_init_struct.Speed=GPIO_SPEED_FREQ_VERY_HIGH;
HAL_GPIO_Init(SDIO_D0_PORT,&gpio_init_struct);

gpio_init_struct.Pin=SDI0_D1_PIN;
HAL_GPIO_Init(SDIO_D1_PORT,&gpio_init_struct);

gpio_init_struct.Pin=SDI0_D2_PIN;
HAL_GPIO_Init(SDIO_D2_PORT,&gpio_init_struct);

gpio_init_struct.Pin=SDI0_D3_PIN;
HAL_GPIO_Init(SDIO_D3_PORT,&gpio_init_struct);

gpio_init_struct.Pin=SDI0_SCK_PIN;
HAL_GPIO_Init(SDIO_SCK_PORT,&gpio_init_struct);

gpio_init_struct.Pin=SDI0_CMD_PIN;
HAL_GPIO_Init(SDIO_CMD_PORT,&gpio_init_struct);
}


/**
 * @brief sd初始化
 * 
 */
SD_HandleTypeDef hsd;
uint8_t sd_init(){

hsd.Instance=SDIO;            //sd基地址
hsd.Init.BusWide=SDIO_BUS_WIDE_1B;            //总线位宽
hsd.Init.ClockBypass=SDIO_CLOCK_BYPASS_DISABLE;        //分频旁路
hsd.Init.ClockDiv=SDIO_TRANSFER_CLK_DIV;           //时钟分频
hsd.Init.ClockEdge=SDIO_CLOCK_EDGE_RISING;          //采样边沿
hsd.Init.ClockPowerSave=SDIO_CLOCK_POWER_SAVE_DISABLE;     //空闲时关闭时钟
hsd.Init.HardwareFlowControl=SDIO_HARDWARE_FLOW_CONTROL_DISABLE;	
	
if(HAL_SD_Init(&hsd)!=HAL_OK){
    return 1;
}
if(HAL_SD_ConfigWideBusOperation(&hsd, SDIO_BUS_WIDE_4B)!=HAL_OK)//切换4bit
{
    return 2;
}
return 0;
}
/**
 * @brief 写函数
 * @note  写的最小单位是块sector，1个块等于512字节
 */
uint8_t sd_write_disk(uint8_t *Data,uint32_t Block_begin,uint32_t block_num)
{
	uint32_t t0;
	//写
if (HAL_SD_WriteBlocks(&hsd,(uint8_t*)Data ,Block_begin, block_num, 5000)!=HAL_OK){
	return -1;

}
//等待
    t0 = HAL_GetTick();
    while (HAL_SD_GetCardState(&hsd) != HAL_SD_CARD_TRANSFER)   /* CMD13 查询，期望回到 4 */
    {
        if (HAL_GetTick() - t0 > 2000U)        return 2;   /* 卡编程超时（坏块重试/掉速） */
        if (hsd.ErrorCode != HAL_SD_ERROR_NONE) return 3;   /* CMD13 本身失败 */
    }

return HAL_OK;
}

/**
 * @brief 读函数
 * @note  读的最小单位是块sector，1个块等于512字节
 */
 uint8_t sd_read_disk(uint8_t *buffer,uint32_t block_begin,uint32_t block_num){
    uint32_t t0;
    if(HAL_SD_ReadBlocks(&hsd, (uint8_t *)buffer,block_begin, block_num, 5000)!=HAL_OK){
        return -1;
    }
    t0=HAL_GetTick();
        while (HAL_SD_GetCardState(&hsd) != HAL_SD_CARD_TRANSFER)   
    {
        if (HAL_GetTick() - t0 > 2000U)         return 2;   /* 卡编程超时（坏块重试/掉速） */
        if (hsd.ErrorCode != HAL_SD_ERROR_NONE) return 3;   /* CMD13 本身失败 */
    }
    return HAL_OK;
 }

// void sd_earse_disk(){
// 	HAL_StatusTypeDef HAL_SD_Erase(&hsd, uint32_t BlockStartAdd, uint32_t BlockEndAdd);
// }
















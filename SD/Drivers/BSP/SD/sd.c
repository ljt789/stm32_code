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

uint8_t sd_init(){
SD_HandleTypeDef hsd;
hsd.Instance=;
	
HAL_SD_Init(&hsd);
}
#ifndef __SD_H
#define __SD_H

#include "stm32f4xx_hal.h"
extern SD_HandleTypeDef hsd;

#define SDI0_CLK_ENABLE()             do{  __HAL_RCC_SDIO_CLK_ENABLE();  }while(0)   


#define SDIO_D0_PORT                  GPIOC
#define SDI0_D0_PIN                   GPIO_PIN_8
#define SDI0_D0_CLK_ENABLE()          do{ __HAL_RCC_GPIOC_CLK_ENABLE(); }while(0)   

#define SDIO_D1_PORT                  GPIOC
#define SDI0_D1_PIN                   GPIO_PIN_9
#define SDI0_D1_CLK_ENABLE()          do{ __HAL_RCC_GPIOC_CLK_ENABLE(); }while(0)   

#define SDIO_D2_PORT                  GPIOC
#define SDI0_D2_PIN                   GPIO_PIN_10
#define SDI0_D2_CLK_ENABLE()          do{ __HAL_RCC_GPIOC_CLK_ENABLE(); }while(0)   

#define SDIO_D3_PORT                  GPIOC
#define SDI0_D3_PIN                   GPIO_PIN_11
#define SDI0_D3_CLK_ENABLE()          do{ __HAL_RCC_GPIOC_CLK_ENABLE(); }while(0)   


#define SDIO_SCK_PORT                  GPIOC
#define SDI0_SCK_PIN                   GPIO_PIN_12
#define SDI0_SCK_CLK_ENABLE()          do{ __HAL_RCC_GPIOC_CLK_ENABLE(); }while(0)   

#define SDIO_CMD_PORT                   GPIOD
#define SDI0_CMD_PIN                   GPIO_PIN_2
#define SDI0_CMD_CLK_ENABLE()          do{ __HAL_RCC_GPIOD_CLK_ENABLE(); }while(0)   



uint8_t sd_init();
uint8_t sd_write_disk(uint8_t *Data,uint32_t Block_begin,uint32_t block_num);
uint8_t sd_read_disk(uint8_t *buffer,uint32_t block_begin,uint32_t block_num);

#endif
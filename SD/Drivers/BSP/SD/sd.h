#ifndef __SD_H
#define __SD_H


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

#endif
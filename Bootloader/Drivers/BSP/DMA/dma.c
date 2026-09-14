#include "stm32f4xx_hal.h"
#include "dma.h"
#include "./SYSTEM/usart/usart.h"

DMA_HandleTypeDef hdma_OTA; //usart1 接收dma的句柄

void ota_dma_init(){
__HAL_RCC_DMA2_CLK_ENABLE();  

hdma_OTA.Instance= DMA2_Stream2;              //DMA stram
hdma_OTA.Init.Channel=DMA_CHANNEL_4;
hdma_OTA.Init.Direction=DMA_PERIPH_TO_MEMORY;
	
hdma_OTA.Init.FIFOMode=DMA_FIFOMODE_DISABLE; 
hdma_OTA.Init.FIFOThreshold=DMA_FIFO_THRESHOLD_FULL;  /* FIFO 关闭时被忽略 */
hdma_OTA.Init.MemBurst=DMA_MBURST_SINGLE;   
hdma_OTA.Init.MemDataAlignment=DMA_MDATAALIGN_BYTE;     //内存地址宽度 字节
hdma_OTA.Init.MemInc=DMA_MINC_ENABLE;                   //内存地址递增
hdma_OTA.Init.Mode=DMA_NORMAL;                        //DMA 模式循环传递
//外设配置相关
hdma_OTA.Init.PeriphBurst=DMA_PBURST_SINGLE;
hdma_OTA.Init.PeriphDataAlignment=DMA_PDATAALIGN_BYTE;  //外设传输的宽度 字节
hdma_OTA.Init.PeriphInc=DMA_PINC_DISABLE;               //外设地址不递增
hdma_OTA.Init.Priority=DMA_PRIORITY_MEDIUM;	            //配置优先级
HAL_DMA_Init(&hdma_OTA);                                //初始化DMA

__HAL_LINKDMA(&g_uart1_handle, hdmarx, hdma_OTA);       //绑定DMA与串口
}

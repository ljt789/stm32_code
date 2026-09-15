/**
 ****************************************************************************************************
 * @file        adc.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.3
 * @date        2021-10-27
 * @brief       ADC 驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 探索者 F407开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20211018
 * 第一次发布
 * V1.1 20211018
 * 1,支持ADC单通道DMA采集
 * 2,新增adc_dma_init和adc_dma_enable函数
 * V1.2 20211018
 * 1,支持ADC多通道DMA采集
 * 2,新增adc_nch_dma_init和adc_nch_dma_gpio_init函数
 * V1.3 20211027
 * 1,支持内部温度传感器温度采集
 * 2,新增adc_temperature_init和adc_get_temperature函数
 *
 *****************************************************************************************************
 */

#ifndef __ADC_H
#define __ADC_H

#include "./SYSTEM/sys/sys.h"


/******************************************************************************************/
/* ADC及引脚 定义 */

#define ADC_ADCX_CHY_GPIO_PORT              GPIOA
#define ADC_ADCX_CHY_GPIO_PIN               GPIO_PIN_5
#define ADC_ADCX_CHY_GPIO_CLK_ENABLE()      do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)     /* PA口时钟使能 */

#define ADC_ADCX                            ADC1 
#define ADC_ADCX_CHY                        ADC_CHANNEL_5                                   /* 通道Y,  0 <= Y <= 17 */ 
#define ADC_ADCX_CHY_CLK_ENABLE()           do{ __HAL_RCC_ADC1_CLK_ENABLE(); }while(0)      /* ADC1 时钟使能 */


#define ADC_CH_NUM                          6                                               /* 需要转换的通道数目 */

/* ADC单通道/多通道 DMA采集 DMA数据流相关 定义
 * 注意: 这里我们的通道还是使用上面的定义.
 */
#define ADC_ADCX_DMASx                      DMA2_Stream4
#define ADC_ADCX_DMASx_Chanel               DMA_CHANNEL_0                                   /* ADC1_DMA请求源 */
#define ADC_ADCX_DMASx_IRQn                 DMA2_Stream4_IRQn
#define ADC_ADCX_DMASx_IRQHandler           DMA2_Stream4_IRQHandler

#define ADC_ADCX_DMASx_IS_TC()              ( DMA2->HISR & (1 << 5) )                       /* 判断 DMA2_Stream4 传输完成标志, 这是一个假函数形式,
                                                                                             * 不能当函数使用, 只能用在if等语句里面 
                                                                                             */
#define ADC_ADCX_DMASx_CLR_TC()             do{ DMA2->HIFCR |= 1 << 5; }while(0)            /* 清除 DMA2_Stream4 传输完成标志 */

/******************************************************************************************/

/* ADC连接内部温度传感器通道定义 */

#define ADC_TEMPSENSOR_CHX                  ADC_CHANNEL_TEMPSENSOR

/******************************************************************************************/

void adc_init(void);                                            /* ADC初始化 */
void adc_channel_set(ADC_HandleTypeDef *adc_handle, uint32_t ch, uint32_t rank, uint32_t stime); /* ADC通道设置 */
uint32_t adc_get_result(uint32_t ch);                           /* 获得某个通道值 */
uint32_t adc_get_result_average(uint32_t ch, uint8_t times);    /* 得到某个通道给定次数采样的平均值 */

void adc_dma_init(uint32_t mar);        /* ADC DMA采集初始化 */
void adc_dma_enable( uint16_t ndtr);    /* 使能一次ADC DMA采集传输 */

void adc_nch_dma_init(uint32_t tmr);    /* ADC多通道 DMA采集初始化 */
void adc_nch_dma_gpio_init(void);       /* ADC GPIO 初始化 */

void adc_temperature_init(void);        /* ADC温度采集初始化函数 */
short adc_get_temperature(void);        /* 获取内部温度传感器的温度 */

#endif 




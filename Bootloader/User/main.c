/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2021-10-14
 * @brief       ������ ʵ��
 * @license     Copyright (c) 2020-2032, �������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� ̽���� F407������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 ****************************************************************************************************
 */

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include <at24c02.h>
#include <stmflash.h>
#include <bootloader.h>
#include <ota.h>
#include <dma.h>
ota_flag_t ota_flag;                     //OTA升级相关的标志位
int main(void)
{
	  uint8_t buf[20];
    HAL_Init();                                 /* ��ʼ��HAL�� */
    sys_stm32_clock_init(336, 8, 2, 7);         /* ����ʱ��,168Mhz */
    delay_init(168);                            /* ��ʱ��ʼ�� */
    led_init();                                 /* ��ʼ��LED */
    
    AT24C02_BSP_INIT();
    HAL_Delay(100);


    ota_flag.state=Read_byte_at24c02(0x01);
    if(ota_flag.state==1){                      /*升级标志位*/

      ota_dma_init();
		  usart_init(115200);


      printf("upgradeing.......\r\n");
      printf("earseing.........\r\n");
      flash_erase_sector(5);                      //擦除app区域
      printf("earse finish.........\r\n");
    }
    else if(ota_flag.state==0){                 /*跳转标志位*/

      jump_app(stm32_app_baseaddr);
    }
    
	
    while(1)
    {


 if(ota_flag.state==OTA_PENDING)
 {
          Write_byte_at24c02(0x01,0);
          NVIC_SystemReset(); 
 }
        
        LED0(0); 
        LED1(1);  			/* LED0*/
        HAL_Delay(100);
			  LED0(1); 
        LED1(0);  			/* LED0*/
        HAL_Delay(100);
    }
}


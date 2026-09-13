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
ota_flag_t ota_flag;                     //OTA升级相关的标志位
int main(void)
{
	  uint8_t buf[20];
    HAL_Init();                                 /* ��ʼ��HAL�� */
    sys_stm32_clock_init(336, 8, 2, 7);         /* ����ʱ��,168Mhz */
    delay_init(168);                            /* ��ʱ��ʼ�� */
    led_init();                                 /* ��ʼ��LED */
		usart_init(115200);
    AT24C02_BSP_INIT();
    HAL_Delay(100);

    printf("[OTA] Checking upgrade request...\r\n");
    ota_flag.state=Read_byte_at24c02(0x01);
    if(ota_flag.state==1){                      /*升级标志位*/
      printf("upgradeing.......\r\n");
    }
    else if(ota_flag.state==0){                 /*跳转标志位*/
      printf("jump app\r\n");
      jump_app(0x8020000);
    }
    
	
    while(1)
    {

    }
}


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

int main(void)
{
    HAL_Init();                                 /* ��ʼ��HAL�� */

    sys_stm32_clock_init(336, 8, 2, 7);         /* ����ʱ��,168Mhz */
    delay_init(168);                            /* ��ʱ��ʼ�� */
    led_init();                                 /* ��ʼ��LED */
		usart_init(115200);
    AT24C02_BSP_INIT();
    HAL_Delay(100);
	  Ping_AT24C02();
    while(1)
    {
        LED0(0);                                /* LED0 �� */
        LED1(1);                                /* LED1 �� */
        delay_ms(500);
        LED0(1);                                /* LED0 �� */
        LED1(0);                                /* LED1 �� */
        delay_ms(500);
    }
}


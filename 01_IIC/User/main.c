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
	  Ping_AT24C02();
	  // for(int i=0;i<256;i++){
		// Write_byte_at24c02(i,i);
		// }
		// for(int i=0;i<256;i++){
	  // uint8_t data=Read_byte_at24c02(i);
		// 	printf("addr:%x data:%x",i,data);
		// }
	  
		// if (stmflash_read(0x08008888, buf,4)==flash_read_success){
    //   for(int i=0;i<4;i++){
    //     printf("%x\r\n",buf[i]);
    //   }
    // }
    uint8_t wbuf[4] = {0x11, 0x22, 0x33, 0x44};
    uint8_t rbuf[4] = {0};

    if (stmflash_write(0x080E0000, wbuf, 4) == flash_write_success)
    {
        stmflash_read(0x080E0000, rbuf, 4);
        printf("read: %02X %02X %02X %02X\r\n", rbuf[0], rbuf[1], rbuf[2], rbuf[3]);
    }
    else
    {
        printf("write fail\r\n");
    }
	

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


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
#include <ring_buffer.h>
#include <ota.h>
#define OTA_CMD_BUF_SIZE   256 
ring_buffer_t g_ota_cmd_rb;              //定义环形缓冲区结构体
uint8_t g_ota_cmd_buf[OTA_CMD_BUF_SIZE]; //定义环形缓冲区大小
uint8_t g_ota_cmd[OTA_CMD_BUF_SIZE];     //用于存放升级指令的缓冲区
uint8_t g_ota_cmd_data;                  //从环形缓冲区取单个数据
uint8_t g_ota_cmd_idx=0;                  //记录升级指令个数
static const uint8_t OTA_CMD_TAB[] = {0x0A, 0x0B, 0x0C, 0x11, 0x12, 0xAA};


ota_flag_t ota_flag;                     //OTA升级相关的标志位
int main(void)
{
	  uint8_t buf[20];
    HAL_Init();                                 /* ��ʼ��HAL�� */
    sys_stm32_clock_init(336, 8, 2, 7);         /* ����ʱ��,168Mhz */
    delay_init(168);                            /* ��ʱ��ʼ�� */
    led_init();                                 /* ��ʼ��LED */
	
		usart_init(115200);
    rb_init(&g_ota_cmd_rb, g_ota_cmd_buf, OTA_CMD_BUF_SIZE);
    AT24C02_BSP_INIT();
    HAL_Delay(100);
	  printf("jump end");
    printf("VTOR = %08X\r\n", SCB->VTOR);
    ota_flag.state=0;
	  Write_byte_at24c02(0x01,0);
	  HAL_Delay(100);
	  printf("success jump app\r\n");
    while(1)
    {
        if(rb_get(&g_ota_cmd_rb, &g_ota_cmd_data)!=0){
       
          if(g_ota_cmd_data==OTA_CMD_TAB[0]){
              g_ota_cmd[g_ota_cmd_idx]=g_ota_cmd_data;
              g_ota_cmd_idx  = 1;

          }
          if (g_ota_cmd_idx > 0 && g_ota_cmd_data == OTA_CMD_TAB[g_ota_cmd_idx]){
              g_ota_cmd[g_ota_cmd_idx]=g_ota_cmd_data;
              g_ota_cmd_idx++;
              if (g_ota_cmd_idx >= 6)   /* 收满立刻判完成, 不等下一个字节 */
									{
										printf("正在启动\r\n");
										g_ota_cmd_idx = 0;
										ota_flag.state=OTA_REQUEST;
										printf("at24c02:%x\r\n",Read_byte_at24c02(0x01));
										Write_byte_at24c02(0x01,ota_flag.state);
										printf("at24c02:%x\r\n",Read_byte_at24c02(0x01));
										NVIC_SystemReset();             /* 软复位，不返回 */
                  }
				   
		        }
				   printf("%x  %d\r\n",g_ota_cmd_data,g_ota_cmd_idx);
				 }
         LED0(0);                                /* LED0*/
         HAL_Delay(100);
         LED0(1);   
				   HAL_Delay(100);
}
		}


/**
 ****************************************************************************************************
 * @file        usart.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.1
 * @date        2023-06-05
 * @brief       ���ڳ�ʼ������(һ���Ǵ���1)��֧��printf
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
 * �޸�˵��
 * V1.0 20211014
 * ��һ�η���
 * V1.1 20230605
 * ɾ��USART_UX_IRQHandler()�����ĳ�ʱ�������޸�HAL_UART_RxCpltCallback()
 ****************************************************************************************************
 */

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include <ring_buffer.h>

/* ���ʹ��os,����������ͷ�ļ����� */
#if SYS_SUPPORT_OS
#include "os.h"                               /* os ʹ�� */
#endif

/******************************************************************************************/
/* �������´���, ֧��printf����, ������Ҫѡ��use MicroLIB */

#if 1
#if (__ARMCC_VERSION >= 6010050)                    /* ʹ��AC6������ʱ */
__asm(".global __use_no_semihosting\n\t");          /* ������ʹ�ð�����ģʽ */
__asm(".global __ARM_use_no_argv \n\t");            /* AC6����Ҫ����main����Ϊ�޲�����ʽ�����򲿷����̿��ܳ��ְ�����ģʽ */

#else
/* ʹ��AC5������ʱ, Ҫ�����ﶨ��__FILE �� ��ʹ�ð�����ģʽ */
#pragma import(__use_no_semihosting)

struct __FILE
{
    int handle;
    /* Whatever you require here. If the only file you are using is */
    /* standard output using printf() for debugging, no file handling */
    /* is required. */
};

#endif

/* ��ʹ�ð�����ģʽ��������Ҫ�ض���_ttywrch\_sys_exit\_sys_command_string����,��ͬʱ����AC6��AC5ģʽ */
int _ttywrch(int ch)
{
    ch = ch;
    return ch;
}

/* ����_sys_exit()�Ա���ʹ�ð�����ģʽ */
void _sys_exit(int x)
{
    x = x;
}

char *_sys_command_string(char *cmd, int len)
{
    return NULL;
}

/* FILE �� stdio.h���涨��. */
FILE __stdout;

/* �ض���fputc����, printf�������ջ�ͨ������fputc����ַ��������� */
int fputc(int ch, FILE *f)
{
    while ((USART1->SR & 0X40) == 0);               /* �ȴ���һ���ַ�������� */

    USART1->DR = (uint8_t)ch;                       /* ��Ҫ���͵��ַ� ch д�뵽DR�Ĵ��� */
    return ch;
}
#endif
/***********************************************END*******************************************/
    
#if USART_EN_RX                                     /* ���ʹ���˽��� */

/* ���ջ���, ���USART_REC_LEN���ֽ�. */
uint8_t g_usart_rx_buf[USART_REC_LEN];

/*  ����״̬
 *  bit15��      ������ɱ�־
 *  bit14��      ���յ�0x0d
 *  bit13~0��    ���յ�����Ч�ֽ���Ŀ
*/
uint16_t g_usart_rx_sta = 0;

uint8_t g_rx_buffer[RXBUFFERSIZE];                  /* HAL接收缓冲区*/

UART_HandleTypeDef g_uart1_handle;                  /* UART结构体*/


/**
 * @brief       串口的初始化
 * @param       baudrate: 串口波特率
 * @note        
 *             
 * @retval      NULL
 */
void usart_init(uint32_t baudrate)
{
    g_uart1_handle.Instance = USART_UX;                         /* USART1 */
    g_uart1_handle.Init.BaudRate = baudrate;                    /* 波特率*/
    g_uart1_handle.Init.WordLength = UART_WORDLENGTH_8B;        /* 数据位 */
    g_uart1_handle.Init.StopBits = UART_STOPBITS_1;             /* 停止位*/
    g_uart1_handle.Init.Parity = UART_PARITY_NONE;              /* 校验位 */
    g_uart1_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;        /* 硬件流 */
    g_uart1_handle.Init.Mode = UART_MODE_TX_RX;                 /* 开启发送接收模式 */
    HAL_UART_Init(&g_uart1_handle);                             /* 初始化串口结构体 */
    
    /* 开启串口接收中断 */
    HAL_UART_Receive_IT(&g_uart1_handle, (uint8_t *)g_rx_buffer, RXBUFFERSIZE);
}

/**
 * @brief       串口硬件初始化
 * @param       huart: UART配置结构体
 * @note        �˺����ᱻHAL_UART_Init()����
 *              ���ʱ��ʹ�ܣ��������ã��ж�����
 * @retval      NULL
 */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef gpio_init_struct;
    if(huart->Instance == USART_UX)                             /* ����Ǵ���1�����д���1 MSP��ʼ�� */
    {
        USART_UX_CLK_ENABLE();                                  /* 开启串口1时钟 */
        USART_TX_GPIO_CLK_ENABLE();                             /* 开启发送引脚的时钟 */
        USART_RX_GPIO_CLK_ENABLE();                             /* 开启接收引脚的时钟 */

        gpio_init_struct.Pin = USART_TX_GPIO_PIN;               /* 发送引脚引脚编号 */
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;                /* 复用推挽 */
        gpio_init_struct.Pull = GPIO_PULLUP;                    /* 开启上拉 */
        gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;          /* 高速时钟*/
        gpio_init_struct.Alternate = USART_TX_GPIO_AF;          /* 选择复用给串口1的发送引脚 */
        HAL_GPIO_Init(USART_TX_GPIO_PORT, &gpio_init_struct);   /* 初始化引脚 */

        gpio_init_struct.Pin = USART_RX_GPIO_PIN;               /* 接收的引脚编号 */
        gpio_init_struct.Alternate = USART_RX_GPIO_AF;          /* 选择复用给串口1的接收引脚 */
        HAL_GPIO_Init(USART_RX_GPIO_PORT, &gpio_init_struct);   /* 初始化引脚*/

#if USART_EN_RX
        HAL_NVIC_EnableIRQ(USART_UX_IRQn);                      /* 开启NVIC中断*/
        HAL_NVIC_SetPriority(USART_UX_IRQn, 3, 3);              /* 设置中断优先级 */
#endif
    }
}


/**
 * @brief       穿口接收中断
 * @param       huart: UART�������ָ��
 * @retval      ��
 * @note        升级请求协议 0x0a  0x0b 0x0c 0x11 0x12 0xaa
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART_UX)             /*  */
    {
        
        rb_put(&g_ota_cmd_rb, g_rx_buffer[0]);
        HAL_UART_Receive_IT(&g_uart1_handle, (uint8_t *)g_rx_buffer, RXBUFFERSIZE);
    }
}

/**
 * @brief       ����1�жϷ�����
 * @param       ��
 * @retval      ��
 */
void USART_UX_IRQHandler(void)
{ 
#if SYS_SUPPORT_OS                              /* ʹ��OS */
    OSIntEnter();    
#endif

    HAL_UART_IRQHandler(&g_uart1_handle);       /* ����HAL���жϴ������ú��� */

#if SYS_SUPPORT_OS                              /* ʹ��OS */
    OSIntExit();
#endif
}

#endif


 

 





/**
 ****************************************************************************************************
 * @file        ring_buffer.c
 * @brief       环形缓冲区(FIFO)实现
 * @note        适用于"一个中断里存, 一个主循环里取"的场景, 两边都不需要关中断
 ****************************************************************************************************
 *
 * 用法示例:
 *
 *   static uint8_t       rx_buf[512];
 *   static ring_buffer_t rb_rx;
 *
 *   rb_init(&rb_rx, rx_buf, sizeof(rx_buf));         // 初始化, 容量 512
 *
 *   //串口接收中断里(生产者)
 *   void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
 *   {
 *       rb_put(&rb_rx, g_rx_buffer[0]);              // 收到一个字节就存进来
 *       HAL_UART_Receive_IT(huart, g_rx_buffer, 1);  // 重新挂载接收
 *   }
 *
 *   //主循环里(消费者)
 *   uint8_t ch;
 *   while (rb_get(&rb_rx, &ch))
 *   {
 *       //处理 ch
 *   }
 */

#include "ring_buffer.h"

/**
 * @brief  初始化环形缓冲区
 * @param  rb   : 句柄
 * @param  buf  : 数据区, 由调用者提供
 * @param  size : 容量, 必须是 2 的幂(16/32/64/128/256/512/1024...)
 * @retval 无
 */
void rb_init(ring_buffer_t *rb, uint8_t *buf, uint16_t size)
{
    rb->buf  = buf;
    rb->size = size;
    rb->mask = (uint16_t)(size - 1U);       /* 只有 size 是 2 的幂, 这句才成立 */
    rb->head = 0U;
    rb->tail = 0U;
}

/**
 * @brief  清空缓冲区(丢弃所有未读数据)
 * @param  rb : 句柄
 * @retval 无
 * @note   只让 tail 追上 head, 不要把 head 和 tail 都清 0
 *         否则会破坏写方那边的计数器
 */
void rb_clear(ring_buffer_t *rb)
{
    rb->tail = rb->head;
}

/**
 * @brief  已存数据的字节数
 * @param  rb : 句柄
 * @retval 已用字节数
 * @note   head/tail 是自由计数的, unsigned 减法本身就能正确处理回绕
 */
uint16_t rb_used(const ring_buffer_t *rb)
{
    return (uint16_t)(rb->head - rb->tail);
}

/**
 * @brief  还能存多少字节
 * @param  rb : 句柄
 * @retval 剩余空间
 */
uint16_t rb_free(const ring_buffer_t *rb)
{
    return (uint16_t)(rb->size - rb_used(rb));
}

/**
 * @brief  判断是否为空
 * @param  rb : 句柄
 * @retval 1: 空   0: 有数据
 */
uint8_t rb_is_empty(const ring_buffer_t *rb)
{
    return (rb->head == rb->tail) ? 1U : 0U;
}

/**
 * @brief  存入一个字节(生产者调用, 一般在中断里)
 * @param  rb   : 句柄
 * @param  data : 要存入的字节
 * @retval 1: 成功   0: 缓冲区已满, 该字节被丢弃
 */
uint8_t rb_put(ring_buffer_t *rb, uint8_t data)
{
    if ((uint16_t)(rb->head - rb->tail) >= rb->size)   /* 满了 */
    {
        return 0U;
    }

    rb->buf[rb->head & rb->mask] = data;
    rb->head++;                                        /* 数据先写, head 后加 */
    return 1U;
}

/**
 * @brief  存入一串字节(生产者调用)
 * @param  rb  : 句柄
 * @param  src : 源数据
 * @param  len : 想写入的长度
 * @retval 实际写入的字节数(缓冲区满时会比 len 小)
 */
uint16_t rb_write(ring_buffer_t *rb, const uint8_t *src, uint16_t len)
{
    uint16_t n = 0U;

    while (n < len)
    {
        if (rb_put(rb, src[n]) == 0U)
        {
            break;
        }
        n++;
    }

    return n;
}

/**
 * @brief  取出一个字节(消费者调用, 一般在主循环里)
 * @param  rb   : 句柄
 * @param  data : 取出的数据存放处
 * @retval 1: 成功   0: 缓冲区为空
 */
uint8_t rb_get(ring_buffer_t *rb, uint8_t *data)
{
    if (rb->head == rb->tail)                          /* 空了 */
    {
        return 0U;
    }

    *data = rb->buf[rb->tail & rb->mask];
    rb->tail++;                                        /* 数据先取, tail 后加 */
    return 1U;
}

/**
 * @brief  取出一串字节(消费者调用)
 * @param  rb  : 句柄
 * @param  dst : 接收数据的目标缓冲
 * @param  len : 想读取的长度
 * @retval 实际读出的字节数(缓冲区不够时会比 len 小)
 */
uint16_t rb_read(ring_buffer_t *rb, uint8_t *dst, uint16_t len)
{
    uint16_t n = 0U;

    while (n < len)
    {
        if (rb_get(rb, &dst[n]) == 0U)
        {
            break;
        }
        n++;
    }

    return n;
}

/**
 * @brief  看一眼下一个字节, 但不取走
 * @param  rb   : 句柄
 * @param  data : 数据存放处
 * @retval 1: 成功   0: 缓冲区为空
 */
uint8_t rb_peek(const ring_buffer_t *rb, uint8_t *data)
{
    if (rb->head == rb->tail)
    {
        return 0U;
    }

    *data = rb->buf[rb->tail & rb->mask];
    return 1U;
}

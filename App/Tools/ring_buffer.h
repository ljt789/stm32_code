/**
 ****************************************************************************************************
 * @file        ring_buffer.h
 * @brief       环形缓冲区(FIFO), 用于串口接收这类"中断里存, 主循环里取"的场景
 * @note        一个生产者 + 一个消费者时, 两边都不需要关中断
 ****************************************************************************************************
 */

#ifndef __RING_BUFFER_H
#define __RING_BUFFER_H

#include <stdint.h>

/**
 * @brief 环形缓冲区句柄
 * @note  head 只由写方修改, tail 只由读方修改, 互不干扰, 所以不需要关中断保护
 */
typedef struct
{
    uint8_t          *buf;      /* 数据区, 由调用者提供, 生命周期要覆盖整个使用过程 */
    uint16_t          size;     /* 容量, 必须是 2 的幂: 16/32/64/128/256/512/1024... */
    uint16_t          mask;     /* size - 1, 用按位与代替取模运算, 中断里更快 */
    volatile uint16_t head;     /* 写索引, 自由计数不归零, 只由写方修改 */
    volatile uint16_t tail;     /* 读索引, 自由计数不归零, 只由读方修改 */
} ring_buffer_t;

void     rb_init(ring_buffer_t *rb, uint8_t *buf, uint16_t size);
void     rb_clear(ring_buffer_t *rb);
uint16_t rb_used(const ring_buffer_t *rb);
uint16_t rb_free(const ring_buffer_t *rb);
uint8_t  rb_is_empty(const ring_buffer_t *rb);
uint8_t  rb_put(ring_buffer_t *rb, uint8_t data);
uint16_t rb_write(ring_buffer_t *rb, const uint8_t *src, uint16_t len);
uint8_t  rb_get(ring_buffer_t *rb, uint8_t *data);
uint16_t rb_read(ring_buffer_t *rb, uint8_t *dst, uint16_t len);
uint8_t  rb_peek(const ring_buffer_t *rb, uint8_t *data);

#endif

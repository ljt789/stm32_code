/**
 ****************************************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2021-10-27
 * @brief       内部温度传感器 实验
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *

 ****************************************************************************************************
 */

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/ADC/adc.h"
#include <sd.h>
#include <ff.h>
#include "diskio.h"
#include <string.h>

extern SD_HandleTypeDef hsd;
HAL_SD_CardStatusTypeDef pStatus;
uint8_t data[512];
uint8_t buffer[512];

FATFS   fs;              /* 卷对象：f_mount 之后 FatFs 一直要用它，必须全局或 static */
FIL     f;
BYTE    wbuf[512];
BYTE    rbuf[512];
BYTE    work[4096];      /* f_mkfs 的工作缓冲区 —— 注意必须是全局！见下面的说明 */
static void fatfs_test(void)
{
    FRESULT fr;
    UINT    bw, br;
    DWORD   fre_clust = 0;
    FATFS  *fsp = 0;

    /* ---------- 1. 挂载 ---------- */

	
fr = f_mount(&fs, "0:", 1);
    printf("f_mount -> %d\r\n", fr);

    if (fr == FR_NO_FILESYSTEM)
    {
        printf("no filesystem, formatting...\r\n");
        fr = f_mkfs("0:", 0, work, sizeof(work));
        printf("f_mkfs -> %d\r\n", fr);
        if (fr == FR_OK) fr = f_mount(&fs, "0:", 1);
    }
    if (fr != FR_OK) { printf("mount failed, abort\r\n"); return; }
		printf("fs_type=%u (1=FAT12 2=FAT16 3=FAT32), %u sectors/cluster\r\n",
           (unsigned)fs.fs_type, (unsigned)fs.csize);

    if (f_getfree("0:", &fre_clust, &fsp) == FR_OK)
    {
        printf("total=%lu KB, free=%lu KB\r\n",
               (unsigned long)(fsp->n_fatent - 2) * fsp->csize / 2,
               (unsigned long)fre_clust * fsp->csize / 2);
    }

        /* ---------- 3. 写文件 ---------- */
    memset(wbuf, 0x55, sizeof(wbuf));
    fr = f_open(&f, "0:/atk.txt", FA_CREATE_ALWAYS | FA_WRITE);
    printf("f_open(w) -> %d\r\n", fr);
    if (fr != FR_OK) return;

    fr = f_write(&f, wbuf, sizeof(wbuf), &bw);
    printf("f_write -> %d, wrote %u bytes\r\n", fr, bw);
    f_close(&f);                 /* 一定要关，否则目录项和数据可能还在缓存里 */

    /* ---------- 4. 读回比对 ---------- */
    memset(rbuf, 0, sizeof(rbuf));
    fr = f_open(&f, "0:/atk.txt", FA_READ);
    printf("f_open(r) -> %d\r\n", fr);
    if (fr != FR_OK) return;

    fr = f_read(&f, rbuf, sizeof(rbuf), &br);
    f_close(&f);

    if (fr == FR_OK && br == sizeof(rbuf) && memcmp(wbuf, rbuf, sizeof(rbuf)) == 0)
        printf("VERIFY OK\r\n");
    else
        printf("VERIFY FAIL: fr=%d br=%u\r\n", fr, br);

}
int main(void)
{
   
    HAL_Init();                             /* ???HAL? */
    sys_stm32_clock_init(336, 8, 2, 7);     /* ????,168Mhz */
    delay_init(168);                        /* ????? */
    usart_init(115200);                     /* ??????115200 */
    led_init();                             /* ???LED */

    fatfs_test();                      /* ??? f_mount */

    while (1)
    {

    
        
        LED0_TOGGLE();  /* LED0闪烁,提示程序运行 */
        delay_ms(500);
    }
}

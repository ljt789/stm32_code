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
extern SD_HandleTypeDef hsd;
HAL_SD_CardStatusTypeDef pStatus;
uint8_t data[512];
int main(void)
{
    short temp;

    HAL_Init();                             /* 初始化HAL库 */
    sys_stm32_clock_init(336, 8, 2, 7);     /* 设置时钟,168Mhz */
    delay_init(168);                        /* 延时初始化 */
    usart_init(115200);                     /* 串口初始化为115200 */
    led_init();                             /* 初始化LED */
    uint8_t status=sd_init();
	  if(status==0){
	       HAL_SD_CardInfoTypeDef   info;
    HAL_SD_CardStatusTypeDef st;
    uint32_t c = SDIO->CLKCR;
    uint64_t bytes;

    /* ① 核对主机侧配置：期望 CLKCR=0x00000900(CLKEN=1, WIDBUS=1 → 4-bit, CLKDIV=0 → 24MHz) */
    printf("CLKCR=0x%08X CLKDIV=%u CLKEN=%u WIDBUS=%u\r\n",
           (unsigned)c, (unsigned)(c & 0xFFU), (unsigned)((c >> 8) & 1U), (unsigned)((c >> 11) & 3U));

    /* ② 容量：来自 CSD(CMD9)，这才是权威数据 */
    HAL_SD_GetCardInfo(&hsd, &info);
    bytes = (uint64_t)info.LogBlockNbr * (uint64_t)info.LogBlockSize;
    printf("CardType=%u(0=SDSC,1=SDHC/SDXC) BlockNbr=%u BlockSize=%u LogBlockNbr=%u\r\n",
           (unsigned)info.CardType, (unsigned)info.BlockNbr,
           (unsigned)info.BlockSize, (unsigned)info.LogBlockNbr);
    printf("Capacity = %u MB (%u.%02u GB)\r\n",
           (unsigned)(info.LogBlockNbr >> 11U),
           (unsigned)(bytes / 1000000000ULL),
           (unsigned)((bytes % 1000000000ULL) / 10000000ULL));

    /* ③ ACMD13 的那些字段用在这里才对 */
    if (HAL_SD_GetCardStatus(&hsd, &st) == HAL_OK)   /* ← 别忘了判断返回值，你这次也没判断 */
    {
        printf("DataBusWidth=%u(0=1bit,2=4bit) ProtectedArea=%u B SpeedClass=%u AU_SIZE=%u EraseSize=%u\r\n",
               (unsigned)st.DataBusWidth, (unsigned)st.ProtectedAreaSize,
               (unsigned)st.SpeedClass, (unsigned)st.AllocationUnitSize, (unsigned)st.EraseSize);
    }
		}
		
		 memset(data,0xff,512);
     status=sd_write_disk(data,1,1);
		 if(status==HAL_OK){
		      printf("写入成功\r\n");
		  }
		 else{
		 printf("error status:%d\r\n",status);}

    while (1)
    {

    
        
        LED0_TOGGLE();  /* LED0闪烁,提示程序运行 */
        delay_ms(250);
    }
}

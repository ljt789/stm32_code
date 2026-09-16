#include <ff.h>
#include <diskio.h>

#include <sd.h>

/*
签名类型：
typedef unsigned char  BYTE;    
typedef unsigned int   UINT;     
typedef uint32_t       DWORD;    
typedef DWORD          LBA_t;    
*/
#define DEV_SD     0
static volatile DSTATUS Stat =STA_NOINIT;  /*卡状态的标志*/

/**
 * @brief 将物理驱动器从0带起来开时钟、配 GPIO、发 CMD0/CMD8/ACMD41 识别卡、读 CSD、切总线宽度……
 * @param BYTE pdrv:物理驱动器号
 */
DSTATUS disk_initialize (BYTE pdrv){
    if (pdrv!=DEV_SD)
    {
        return STA_NOINIT;
    }

    if(Stat & STA_NOINIT){
        if(sd_init()!=HAL_OK){
            Stat|=STA_NOINIT;
            return Stat;
        }
        Stat&=(DSTATUS)~STA_NOINIT;
    }
    
    return Stat;
}

/**
 * @return #define STA_NOINIT   0x01  
        // #define STA_NODISK   0x02   
        // #define STA_PROTECT  0x04  
 */
DSTATUS disk_status (BYTE pdrv){
    if (pdrv!=DEV_SD)
    {
        return STA_NOINIT;
    }
    return Stat;

}
/**
 * @brief 从第 sector 个扇区开始，连续读 count 个扇区，共 count × 512 字节，放进 buff
 */
DRESULT disk_read (BYTE pdrv, BYTE* buff, LBA_t sector, UINT count){
    if(pdrv !=DEV_SD ||count==0){
        return RES_PARERR;
    }
    if (Stat & STA_NOINIT)
    {
        return RES_NOTRDY;
    }
    if (sd_read_disk((uint8_t *)buff,(uint32_t)sector,(uint32_t)count)!=HAL_OK)
    {
        return RES_ERROR;
    }
    return RES_OK;
}
/**
 * @brief 把 buff 里的 count × 512 字节写到从第 sector 个扇区开始的连续区域。
 */
DRESULT disk_write (BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count){
    if(pdrv !=DEV_SD ||count==0){
        return RES_PARERR;
    }
    if (Stat & STA_NOINIT)
    {
        return RES_NOTRDY;
    }
    if (Stat & STA_PROTECT)
    {
        return RES_WRPRT;                   /* 目前没做写保护检测，走不到这里 */
    }
    if(sd_write_disk((uint8_t *)buff,(uint32_t)sector,(uint32_t )count)!=HAL_OK){
        return RES_ERROR;
    }

    return RES_OK;

}
/**
 * @brief 杂项控制
 */
DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void* buff){

    if (pdrv != DEV_SD)
    {
        return RES_PARERR;
    }
    if (Stat & STA_NOINIT)
    {
        return RES_NOTRDY;
    }
   switch (cmd)
    {
        case CTRL_SYNC:
            /* sync_fs() 每次同步都调。sd_write_disk 是阻塞式、写完才返回，
               没有写缓存要刷，所以直接返回 OK 是安全的。
               （将来若改成 DMA 异步写，这里必须等 DMA 真正完成）*/
            return RES_OK;

        case GET_SECTOR_COUNT:
            /* f_mkfs() 要用。这个值来自 CMD9 读到的 CSD，
               HAL_SD_Init() 成功后就已经算好放在 hsd.SdCard 里了 */
            *(LBA_t *)buff = (LBA_t)hsd.SdCard.LogBlockNbr;
            return RES_OK;

        case GET_BLOCK_SIZE:
            /* f_mkfs() 要用的擦除块大小，单位 = 扇区。
               填 1 表示"不知道"，FatFs 会按 1 处理，最保险 */
            *(DWORD *)buff = 1;
            return RES_OK;

        default:
            return RES_PARERR;
    }

}
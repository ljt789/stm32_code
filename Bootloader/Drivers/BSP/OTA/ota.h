#ifndef __OTA_H
#define __OTA_H
#include <stdint.h>
#define OTA_FLAG_MAGIC     0x4F544131U   /* "OTA1"，用来判断这块区域有没有被写过 */
#define OTA_FLAG_VER       0x0001U       /* 结构体版本，以后加字段用 */

typedef enum {
    OTA_IDLE        = 0x00,   /* 空闲，正常启动 */
    OTA_REQUEST     = 0x01,   /* App 收到命令，等重启进 Bootloader */
    OTA_DOWNLOADING = 0x02,   /* Bootloader 正在收数据、写 flash */
    OTA_VERIFYING   = 0x03,   /* 数据收完，正在校验 */
    OTA_PENDING     = 0x04,   /* 校验通过、已跳转，等 App 回报"我起来了" */
    OTA_CONFIRMED   = 0x05,   /* 新固件自检通过，升级彻底完成 */
    OTA_FAILED      = 0x06,   /* 失败，按重试次数决定重来还是留守 */
} ota_state_e;

typedef struct {
    uint32_t magic;        /* +0  未写过是 0xFFFFFFFF，写过是 OTA_FLAG_MAGIC */
    uint16_t struct_ver;   /* +4  结构体版本 */
    uint8_t  state;        /* +6  见上面的状态机 */
    uint8_t  boot_mode;    /* +7  1 = 本次上电强制进 Bootloader */
    uint32_t fw_version;   /* +8  目标版本号，用来判断"是不是同一版" */
    uint32_t fw_size;      /* +12 固件字节数 */
    uint32_t fw_crc32;     /* +16 整包 CRC32，服务器算好发过来 */
    uint32_t fw_addr;      /* +20 写入起始地址，现在是 0x08020000 */
    uint32_t fw_written;   /* +24 已经写进去多少字节，断点续传用 */
    uint16_t retry_cnt;    /* +28 已重试次数 */
    uint16_t retry_max;    /* +30 上限，建议 3 */
    uint32_t hdr_crc32;    /* +32 前面这 32 个字节的 CRC32，校验本结构体本身 */
} ota_flag_t;

#endif
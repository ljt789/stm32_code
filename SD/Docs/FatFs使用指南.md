# FatFs 使用指南（结合本项目）

> 适用范围：`E:\MCU_TEST_CODE\SD` 工程
> FatFs 版本：R0.15（`FFCONF_DEF 80386`）
> 硬件：STM32F407 + SDIO + SD 卡（SPI 模式不适用本文的 `sd.c` 驱动）

---

## 目录

1. [整体架构](#1-整体架构)
2. [本项目的裁剪配置](#2-本项目的裁剪配置)
3. [移植对接：diskio.c](#3-移植对接diskioc)
4. [核心数据结构](#4-核心数据结构)
5. [标准使用流程](#5-标准使用流程)
6. [常用 API 详解](#6-常用-api-详解)
7. [目录操作与文件遍历](#7-目录操作与文件遍历)
8. [格式化与挂载](#8-格式化与挂载)
9. [常见错误码速查](#9-常见错误码速查)
10. [常见坑与注意事项](#10-常见坑与注意事项)

---

## 1. 整体架构

```
┌─────────────────────────────────────────────┐
│  应用层  User/main.c                          │
│  f_open / f_write / f_read / f_close ...     │
├─────────────────────────────────────────────┤
│  FatFs 核心层  Middlewares/FATFS/             │
│  ff.c / ff.h / ffconf.h / ffunicode.c        │
│  （纯 C，与硬件无关，不要改动）                 │
├─────────────────────────────────────────────┤
│  对接层  Middlewares/FATFS/diskio.c           │
│  disk_initialize / disk_read / disk_write …  │
│  （移植时唯一需要自己写的文件）                 │
├─────────────────────────────────────────────┤
│  硬件驱动层  Drivers/BSP/SD/sd.c              │
│  sd_init / sd_read_disk / sd_write_disk      │
│  （基于 STM32 HAL 的 SDIO 驱动）              │
└─────────────────────────────────────────────┘
```

FatFs 是纯软件的文件系统模块，只认"扇区"这个抽象概念。
它通过 5 个约定好的函数向下要数据，你把这 5 个函数接到 SD 卡驱动上，移植就完成了。

---

## 2. 本项目的裁剪配置

`Middlewares/FATFS/ffconf.h` 中的关键项（改动配置后必须重新编译全工程）：

| 配置项 | 值 | 含义 |
|---|---|---|
| `FF_FS_READONLY` | 0 | 可读可写 |
| `FF_FS_MINIMIZE` | 0 | 基本 API 全部保留 |
| `FF_USE_FIND` | 1 | 启用 `f_findfirst/f_findnext` |
| `FF_USE_MKFS` | 1 | 启用格式化 `f_mkfs()` |
| `FF_USE_LABEL` | 1 | 启用卷标读写 |
| `FF_USE_STRFUNC` | 1 | 启用 `f_gets/f_puts/f_printf` |
| `FF_CODE_PAGE` | 936 | 简体中文 GBK 码页，支持中文文件名 |
| `FF_USE_LFN` | 1 | 启用长文件名（缓冲区在**栈**上分配） |
| `FF_MAX_LFN` | 255 | 长文件名最大 255 字符 |
| `FF_LFN_UNICODE` | 0 | 应用层用 ANSI/GBK 字符串（`char*`），不是 `wchar_t` |
| `FF_FS_TINY` | 1 | Tiny 模式：所有 FIL 共享 FATFS 里的一个扇区缓冲，省 RAM |
| `FF_MIN_SS / FF_MAX_SS` | 512 | 扇区固定 512 字节 |
| `FF_FS_EXFAT` | 0 | 不支持 exFAT，只支持 FAT12/16/32 |
| `FF_VOLUMES` | 1 | 只挂 1 个卷，路径前缀 `"0:"` |
| `FF_FS_RPATH` | 0 | 不支持相对路径，路径一律从根写起 |
| `FF_FS_NORTC` | 1 | 无 RTC，文件时间戳用固定日期 |
| `FF_FS_REENTRANT` | 0 | 无重入保护（裸机单任务使用） |

**注意**：`FF_USE_LFN = 1` 是"缓冲区放在栈上"，每层函数调用要额外消耗约 `(FF_MAX_LFN+1)*2` 字节栈。如果栈紧张，可改成 `2`（放静态区）或 `3`（放堆）。

---

## 3. 移植对接：diskio.c

FatFs 要求实现 5 个函数，本项目实现见 `Middlewares/FATFS/diskio.c`：

| 函数 | 作用 | 本项目实现 |
|---|---|---|
| `disk_initialize(pdrv)` | 初始化物理介质 | 调 `sd_init()`（CMD0/CMD8/ACMD41 识别卡、读 CSD、切总线宽度） |
| `disk_status(pdrv)` | 返回卡状态 | 返回 `STA_NOINIT / STA_NODISK / STA_PROTECT` 标志 |
| `disk_read(pdrv, buff, sector, count)` | 读 count 个扇区 | 转调 `sd_read_disk()` |
| `disk_write(pdrv, buff, sector, count)` | 写 count 个扇区 | 转调 `sd_write_disk()` |
| `disk_ioctl(pdrv, cmd, buff)` | 杂项控制 | 见下 |

`disk_ioctl` 需要处理的命令：

- `CTRL_SYNC`：刷缓存。本项目 `sd_write_disk` 是阻塞写，直接返回 `RES_OK`。
  **将来改成 DMA 异步写时，这里必须等 DMA 真正完成再返回。**
- `GET_SECTOR_COUNT`：总扇区数，`f_mkfs` 要用。取 `hsd.SdCard.LogBlockNbr`。
- `GET_BLOCK_SIZE`：擦除块大小（单位：扇区）。填 1 表示未知，FatFs 按 1 处理。

---

## 4. 核心数据结构

### 4.1 FATFS —— 卷对象（逻辑盘）

```c
FATFS fs;   /* 每挂载一个卷需要一个，必须全局或 static */
```

f_mount 之后 FatFs 一直要用它，**绝不能是局部变量**。
本项目开了 `FF_FS_TINY`，共享的 512 字节扇区缓冲也在这个对象里。

### 4.2 FIL —— 文件对象

```c
FIL f;     /* 每打开一个文件需要一个 */
```

包含文件指针、大小、读写位置等。打开期间有效，`f_close` 后可复用。

### 4.3 FRESULT —— 返回码

所有 `f_xxx()` 函数返回 `FRESULT`，`FR_OK == 0` 表示成功。

---

## 5. 标准使用流程

### 5.1 写文件（本项目 main.c 的实际流程）

```c
FATFS fs;              /* 卷对象：必须全局或 static */
FIL    f;
BYTE   work[4096];     /* f_mkfs 的工作缓冲区 —— 也必须全局！ */
FRESULT fr;
UINT    bw;

/* ① 挂载（opt=1 立即挂载，能立刻发现卡是否可用） */
fr = f_mount(&fs, "0:", 1);

/* ② 挂载失败 → 先格式化再重挂（仅出厂新卡/卡损坏时走到这里） */
if (fr != FR_OK) {
    fr = f_mkfs("0:", 0, work, sizeof(work));  /* 0 = 自动选 FAT 类型 */
    if (fr == FR_OK)
        fr = f_mount(&fs, "0:", 1);
}

/* ③ 打开/创建文件 */
fr = f_open(&f, "0:/atk.txt", FA_CREATE_ALWAYS | FA_WRITE);

/* ④ 写数据 */
f_write(&f, wbuf, sizeof(wbuf), &bw);

/* ⑤ 关闭（会自动把剩余缓存落盘） */
f_close(&f);
```

### 5.2 读文件

```c
FIL f; UINT br; char rbuf[512];

f_open(&f, "0:/atk.txt", FA_READ);
f_read(&f, rbuf, sizeof(rbuf), &br);   /* br = 实际读到的字节数 */
f_close(&f);
```

### 5.3 追加写

```c
f_open(&f, "0:/log.txt", FA_OPEN_APPEND | FA_WRITE);
f_write(&f, buf, len, &bw);
f_close(&f);
```

`FA_OPEN_APPEND` 打开时读写位置自动移到文件末尾，不用手动 `f_lseek`。

---

## 6. 常用 API 详解

### 6.1 打开模式标志（f_open 第二参）

| 标志 | 含义 |
|---|---|
| `FA_READ` | 读 |
| `FA_WRITE` | 写 |
| `FA_OPEN_EXISTING` | 只打开已有文件（默认） |
| `FA_OPEN_ALWAYS` | 存在则打开，不存在则创建 |
| `FA_CREATE_NEW` | 创建新文件；**已存在则失败**（FR_EXIST） |
| `FA_CREATE_ALWAYS` | 创建并清空；已存在则覆盖为 0 字节 |
| `FA_OPEN_APPEND` | 打开并把指针移到末尾 |

写文件至少要 `FA_WRITE`（可与 `FA_READ` 组合实现读写打开）。

### 6.2 读 / 写 / 定位

```c
FRESULT f_read (FIL* fp, void* buff, UINT btr, UINT* br);
FRESULT f_write(FIL* fp, const void* buff, UINT btw, UINT* bw);
FRESULT f_lseek(FIL* fp, FSIZE_t ofs);   /* 移动读写指针 */
FSIZE_t f_tell(FIL* fp);                  /* 当前指针位置 */
FSIZE_t f_size(FIL* fp);                  /* 文件大小 */
FRESULT f_sync(FIL* fp);                  /* 不关文件，先落盘（防断电丢数据） */
```

要点：

- `f_read/f_write` 的第 4 参返回**实际**读/写字节数。
  `f_write` 返回的 `bw < btw` 只在盘满（`FR_DISK_FULL`）时发生。
- 长期写日志的场景，建议周期性调 `f_sync()`，断电时最多丢最近一段。

### 6.3 文件系统管理

```c
f_close(&f);                        /* 关文件（内部含 sync） */
f_unlink("0:/old.txt");             /* 删除文件（不能删非空目录） */
f_rename("0:/a.txt", "0:/b.txt");   /* 改名/移动 */
f_truncate(&f);                     /* 在当前指针处截断文件 */
f_mkdir("0:/dir1");                 /* 建目录 */
f_stat("0:/a.txt", &fno);           /* 查文件信息，也可用来判断存在性 */
f_getfree("0:", &fre_clust, &fs2);  /* 查剩余空间：fre_clust × fs2.csize × 512 字节 */
f_getlabel("0:", label, &vsn);      /* 读卷标（本项目已启用） */
```

判断文件是否存在的惯用写法：

```c
if (f_stat("0:/cfg.bin", &fno) == FR_OK) { /* 存在 */ }
```

---

## 7. 目录操作与文件遍历

```c
DIR dir; FRESULT fr; FILINFO fno;

fr = f_opendir(&dir, "0:/");              /* 打开根目录 */
while (f_readdir(&dir, &fno) == FR_OK && fno.fname[0]) {
    if (fno.fattrib & AM_DIR)
        printf("[目录] %s\r\n", fno.fname);
    else
        printf("[文件] %s  %lu 字节\r\n", fno.fname, (unsigned long)fno.fsize);
}
f_closedir(&dir);
```

`FILINFO` 常用字段：`fname`（文件名，LFN 开启时是长名）、`fsize`、`fattrib`（`AM_DIR` 目录 / `AM_RDO` 只读）。

本项目启用了 `FF_USE_FIND`，可用通配符过滤：

```c
fr = f_findfirst(&dir, &fno, "0:/", "*.txt");
while (fr == FR_OK && fno.fname[0]) {
    printf("%s\r\n", fno.fname);
    fr = f_findnext(&dir, &fno);
}
f_closedir(&dir);
```

---

## 8. 格式化与挂载

### 8.1 f_mount

```c
FRESULT f_mount(FATFS* fs, const TCHAR* path, BYTE opt);
```

- `opt = 0`：只登记，不碰硬件（延迟挂载，首次文件操作时才真正读卡）。
- `opt = 1`：立即挂载，马上读分区表和 BPB，能**当场**发现"卡没格式化/没插卡"。
- 卸载：`f_mount(&fs, "0:", 0)` 之前应保证所有文件已 `f_close`。

### 8.2 f_mkfs

```c
FRESULT f_mkfs(const TCHAR* path, const MKFS_PARM* opt, void* work, UINT len);
```

- `opt = NULL 或 0`：自动选择 FAT 类型（按卷大小选 FAT16/FAT32；因为 `FF_FS_EXFAT=0`，不会选 exFAT）。
- `work`：工作缓冲区，**建议 ≥ 4096 字节且为全局内存**（本工程 `work[4096]`）。
- 格式化后必须重新 `f_mount` 才能使用。
- 簇大小等高级控制可填 `MKFS_PARM` 结构（`fm_fat32`、`au=0` 自动等），一般用默认即可。

> 只支持 FAT12/16/32。容量 >32GB 的新卡（出厂 exFAT）要么先在 PC 上格成 FAT32，要么手动指定参数用 `f_mkfs` 格 FAT32。

---

## 9. 常见错误码速查

| 错误码 | 含义 | 常见原因 |
|---|---|---|
| `FR_OK` (0) | 成功 | — |
| `FR_DISK_ERR` | 底层读写出错 | `disk_read/write` 失败；接线/时序问题 |
| `FR_NOT_READY` | 介质未就绪 | 没 mount 或 `disk_initialize` 失败 |
| `FR_NO_FILESYSTEM` | 没有文件系统 | 新卡未格式化 → 走 `f_mkfs` 流程 |
| `FR_NO_PATH` | 路径中的目录不存在 | 如 `0:/dir1/a.txt` 但 dir1 不存在 |
| `FR_INVALID_NAME` | 文件名非法 | 名字太长、含非法字符 |
| `FR_EXIST` | 文件已存在 | `FA_CREATE_NEW` 遇到同名文件 |
| `FR_DENIED` | 操作被拒 | 删非空目录、对只读文件写、盘满 |
| `FR_DISK_FULL` | 盘满 | `f_write` 的 `bw < btw` |
| `FR_WRITE_PROTECTED` | 写保护 | `disk_status` 报了 `STA_PROTECT` |

调试建议：每个 `f_xxx()` 的返回值都 `printf` 出来，对照本表定位，效率最高。

---

## 10. 常见坑与注意事项

1. **`FATFS`、`FIL`、`work[]` 必须全局或 static**。
   `FATFS` 在 f_mount 之后一直被引用；`work` 建议直接全局（栈上 4KB 很容易爆）。

2. **路径必须带卷号前缀**（`FF_FS_RPATH=0`）：写 `"0:/xxx.txt"`，不要写 `"/xxx.txt"`。

3. **中文文件名**：码页 936 + LFN 已开启，源文件里的中文字符串要保证以 GBK 编译（Keil 默认即可，注意编辑器保存编码一致）。乱码多半是文件编码和码页不匹配。

4. **文件打开期间不要拔卡**；拔插卡后必须 `f_mount(NULL,...)` 卸载再重新挂载，否则缓存里是旧卡的数据。

5. **写完不一定落盘**：数据先进 FatFs 的扇区缓冲，`f_close` 或 `f_sync` 才保证写到卡上。重要数据写完立刻 `f_sync`。

6. **掉电安全**：Tiny 模式下 FAT 表/数据交替读写时会多一次搬运。对掉电敏感的场景，缩短 `f_sync` 周期、或考虑 `FF_FS_TINY=0` 换性能。

7. **无重入保护**（`FF_FS_REENTRANT=0`）：如果以后上 RTOS，多任务同时访问文件必须自己加互斥锁，或把 `FF_FS_REENTRANT` 改 1 并实现 `ff_req_grant/ff_rel_grant`。

8. **栈消耗**：`FF_USE_LFN=1`（栈上 LFN 缓冲）+ 目录遍历， deepest 调用链可能用到 1KB+ 栈。若出现莫名 HardFault/死机，先检查 MSP 栈大小（Keil startup 文件里的 `Stack_Size`）。

9. **DMA 异步写改造提醒**（diskio.c 中已注释）：`disk_write` 改成 DMA 后，`disk_ioctl` 的 `CTRL_SYNC` 必须等待 DMA 完成，否则 `f_sync/f_close` 返回时数据可能还没真正写入。

---

## 附：快速模板（复制即用）

```c
#include "ff.h"

static FATFS g_fs;                 /* 卷对象 */
static BYTE  g_work[4096];         /* f_mkfs 工作区 */

int sd_fs_init(void)
{
    FRESULT fr = f_mount(&g_fs, "0:", 1);
    if (fr == FR_NO_FILESYSTEM) {              /* 新卡：格式化后重挂 */
        fr = f_mkfs("0:", 0, g_work, sizeof(g_work));
        if (fr == FR_OK) fr = f_mount(&g_fs, "0:", 1);
    }
    return (fr == FR_OK) ? 0 : -1;
}

int sd_write_file(const char *path, const void *buf, UINT len)
{
    FIL f; UINT bw; FRESULT fr;
    fr = f_open(&f, path, FA_CREATE_ALWAYS | FA_WRITE);
    if (fr != FR_OK) return -1;
    fr = f_write(&f, buf, len, &bw);
    f_close(&f);
    return (fr == FR_OK && bw == len) ? 0 : -1;
}
```

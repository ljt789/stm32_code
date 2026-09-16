# FatFs 学习路线（STM32F407 + SD 卡方向）

> 结合本项目实际进度定制：SD 卡初始化、写函数已完成（见提交记录），
> 下一阶段重点是**读函数 → FatFs 移植 → 实战应用**。
> 预计总周期：4~6 周（每天 1~2 小时）。

---

## 总览

| 阶段 | 内容 | 目标 | 预计用时 | 状态 |
|------|------|------|---------|------|
| 0 | 前置知识 | 补齐 SD 卡协议与文件系统概念 | 2~3 天 | 部分完成 |
| 1 | 裸机驱动 SD 卡 | 不用文件系统，直接按扇区读写 | 3~5 天 | 🔶 进行中 |
| 2 | FatFs 入门与移植 | 让 FatFs 在 F407 上跑起来 | 3~4 天 | ⬜ |
| 3 | 核心 API 精通 | 熟练操作文件与目录 | 4~5 天 | ⬜ |
| 4 | 配置与裁剪 | 吃透 ffconf.h，按需裁剪 | 2~3 天 | ⬜ |
| 5 | 性能优化 | 读写速度提升一个量级 | 3~4 天 | ⬜ |
| 6 | 实战项目 | 用真实需求巩固（含 IAP 升级） | 1~2 周 | ⬜ |
| 7 | 原理深入（可选） | 读源码、手写 FAT 解析器 | 不限 | ⬜ |

---

## 阶段 0：前置知识

### 0.1 SD 卡硬件与协议基础
- [ ] SD 卡分类：SD（≤2GB）/ SDHC（≤32GB）/ SDXC（>32GB），寻址方式差异（字节地址 vs 块地址）
- [ ] 接口模式：SPI 模式 vs SDIO 模式（1bit / 4bit），F407 用 **SDIO 4bit**
- [ ] 卡的初始化命令流：`CMD0` → `CMD8` → `ACMD41` → `CMD2` → `CMD3` → `CMD7` → `CMD9`
- [ ] 数据读写命令：`CMD17`（单块读）、`CMD18`（多块读）、`CMD24`（单块写）、`CMD25`（多块写）
- [ ] 扇区概念：固定 512 字节（SDHC/SDXC），理解"逻辑块"与"物理扇区"

### 0.2 文件系统基础概念
- [ ] FAT 家族：FAT12 / FAT16 / FAT32 / exFAT 的区别与适用容量
- [ ] 磁盘布局：MBR（主引导记录）→ 分区表 → DBR（引导扇区）→ FAT 表 → 根目录 → 数据区
- [ ] 核心概念：**簇（Cluster）**、**FAT 链**、**目录项（Directory Entry）**、短文件名 SFN 与长文件名 LFN
- [ ] 推荐工具：装一个 **WinHex** 或用 `dd` 导出前几个扇区，肉眼对照结构图看一遍

### 0.3 C 语言与工程能力自检
- [ ] 结构体、联合体、位域、函数指针（diskio 回调会大量用到）
- [ ] `#define` 条件编译与宏开关（ffconf.h 全是开关）
- [ ] 会用 Keil/MDK 的 Map 文件和调试器看变量、内存

---

## 阶段 1：裸机驱动 SD 卡（当前阶段）

> 目标：**不借助任何文件系统**，用扇区为单位直接读写 SD 卡，为 FatFs 的 diskio 打底。

- [x] SD 卡初始化（SDIO 时钟配置、卡识别、RCA 获取）—— 已完成
- [x] 单扇区写 —— 已完成
- [ ] **单扇区读（CMD17）** ← 下一步
- [ ] 多扇区读（CMD18 + CMD12 停止）
- [ ] 多扇区写（CMD25 + 停止事务 + 忙等待）
- [ ] 擦除（CMD32/33/38，`disk_ioctl` 会用到）
- [ ] 读 CSD/CID 寄存器：容量、块大小、卡速度等级
- [ ] 错误处理与卡状态检测（数据 CRC 错误、命令超时、写保护）
- [ ] **验证方法**：写一个扇区的固定数据，拔卡插读卡器，用 WinHex 查看该扇区内容是否一致；再反向读回对比

> 💡 关键练习：写完之后把写过的扇区**读回来校验**，这一步顺便就把"读"验证了。

---

## 阶段 2：FatFs 入门与移植

### 2.1 认识 FatFs
- [ ] FatFs 是什么：ChaN 写的**通用 FAT 文件系统中间件**，纯 ANSI C，与硬件无关
- [ ] 官网（源码 + 手册，唯一权威来源）：http://elm-chan.org/fsw/ff/
- [ ] 源码目录结构：
  - `ff.c` / `ff.h` —— 文件系统核心（一般不改）
  - `ffconf.h` —— **配置文件（重点）**
  - `diskio.h` —— 硬件层接口声明
  - `integer.h` —— 基本类型定义
  - `ffunicode.c` / `cc936.c` —— Unicode 转换表（长文件名用）

### 2.2 理解分层模型（最重要的一张图）

```
应用层      f_open / f_read / f_write / f_close ...
              │
文件系统层    ff.c（FatFs 核心，平台无关）
              │  调用
硬件抽象层    diskio.c ← 你要实现的 5+1 个函数
              │  调用
驱动层        SD 卡驱动（你的 sd_driver.c，阶段 1 的成果）
```

- [ ] 背下需要实现的 **6 个 diskio 函数**：

| 函数 | 作用 | 对应你的驱动 |
|------|------|-------------|
| `disk_initialize` | 初始化介质 | SD_Init() |
| `disk_status` | 查询介质是否就绪 | 返回 0 即可 |
| `disk_read` | 读扇区 | SD_ReadSectors() |
| `disk_write` | 写扇区 | SD_WriteSectors() |
| `disk_ioctl` | 杂项控制（擦块大小、扇区数、同步等） | 按命令实现 |
| `get_fattime` | 返回当前时间戳 | RTC 或写死一个合法值 |

> ⚠️ 新手三大坑：
> 1. `get_fattime` 返回 0 → 文件时间显示异常，甚至挂载失败，务必返回合法格式值；
> 2. `disk_ioctl` 没实现 `GET_SECTOR_COUNT` 等 → `f_mkfs` 直接失败；
> 3. 扇区大小填错（FF_MIN_SS / FF_MAX_SS）→ 读写错位。

### 2.3 动手移植步骤
- [ ] 把 FatFs 源码（R0.15）加入 MDK 工程，加头文件路径
- [ ] `ffconf.h` 首次最小配置：
  - `FF_FS_READONLY 0`
  - `FF_USE_LFN 1`（栈上缓冲）
  - `FF_CODE_PAGE 936`（中文 GBK，需要 cc936.c）
  - `FF_MIN_SS = FF_MAX_SS = 512`
  - `FF_FS_NORTC 1`（暂无 RTC 时用固定时间）
- [ ] 编写 `diskio.c`，把 6 个函数接到你的 SD 驱动
- [ ] 完成第一套生命周期：
  ```c
  f_mount(&fs, "0:", 1);        // 1 = 立即挂载
  f_mkfs("0:", NULL, work, sizeof(work));  // 新卡需格式化
  f_open(&file, "0:test.txt", FA_CREATE_ALWAYS | FA_WRITE);
  f_write(&file, "hello fatfs", 11, &bw);
  f_close(&file);
  f_mount(NULL, "0:", 0);       // 卸载
  ```
- [ ] **终极验证**：拔卡插电脑，能看到 `test.txt` 且内容正确 🎉

---

## 阶段 3：核心 API 精通

### 3.1 文件读写（务必练熟）
- [ ] 打开模式组合：`FA_READ` / `FA_WRITE` / `FA_OPEN_EXISTING` / `FA_CREATE_ALWAYS` / `FA_OPEN_APPEND` / `FA_CREATE_NEW`
- [ ] `f_read` / `f_write` 的返回值与 `*br` / `*bw` 字节数判断（**两者都要查**）
- [ ] `f_lseek` 随机访问 + `FA_OPEN_APPEND` 从尾部追加（做日志必备）
- [ ] `f_sync`：长写过程中周期性落盘（防掉电丢数据）
- [ ] 格式化 IO：`f_printf` / `f_puts` / `f_gets` / `f_gets` 的行尾处理

### 3.2 目录操作
- [ ] `f_opendir` / `f_readdir` 遍历目录（注意 `fno.fname[0] == 0` 表示读完）
- [ ] `f_mkdir` / `f_unlink` / `f_rename`
- [ ] `f_stat` 获取文件大小、时间、属性
- [ ] `f_getfree` 查询剩余空间
- [ ] 递归遍历整个 SD 卡并打印文件树（经典练习题）

### 3.3 错误码体系
- [ ] 背熟常用 `FR_OK / FR_DISK_ERR / FR_NOT_READY / FR_NO_FILE / FR_NO_FILESYSTEM / FR_INVALID_NAME / FR_DENIED`
- [ ] 每个函数调用后打印返回码 + 行号，养成习惯

---

## 阶段 4：ffconf.h 配置与裁剪

> 吃透这份配置文件 = 吃透 FatFs 的一半。

- [ ] `FF_FS_MINIMIZE`（1/2/3 级裁剪，砍掉哪些功能）
- [ ] `FF_USE_LFN`：0 关闭 / 1 栈缓冲 / 2 堆缓冲 / 3 静态缓冲；占用与栈风险
- [ ] `FF_CODE_PAGE`：936（GBK 中文）vs 437（ASCII）；`FF_UNICODE` 相关（UTF-8/16）
- [ ] `FF_FS_TINY`、`FF_FS_EXFAT`（>32GB 卡必须开）
- [ ] `FF_USE_STRFUNC`、`FF_USE_FIND`、`FF_USE_CHMOD`、`FF_USE_EXPAND`
- [ ] `FF_FS_LOCK`（多文件句柄防冲突）
- [ ] 裁剪前后对比 Code（RO）+ RW 占用，感受每个开关的代价
- [ ] 练习：把 LFN 打开，用中文名建文件，在电脑上验证显示正常

---

## 阶段 5：性能优化

- [ ] **多扇区连续读写**（最大提速点）：确认 `disk_read/disk_write` 支持多扇区，FatFs 内部会自动合并
- [ ] `disk_ioctl` 完整实现：`CTRL_SYNC`、`GET_SECTOR_COUNT`、`GET_BLOCK_SIZE`、`CTRL_TRIM`
- [ ] SDIO **DMA** 收发代替轮询（F407 SDIO 自带 DMA 请求映射）
- [ ] 提高时钟：SDIO CLK 从 400kHz（初始化）提到 24~48MHz 的时机与约束
- [ ] `FF_USE_FASTSEEK 1` + `f_lseek` 建 FAT 链缓存（大文件随机访问）
- [ ] 读写基准测试：裸扇区速度 vs FatFs 文件速度，分别测单块/多块/1MB 大文件
  - 目标参考：4bit @ 24MHz 裸写 ≥ 2MB/s，FatFs 顺序写 ≥ 1.5MB/s
- [ ] `f_mkfs` 的 `au`（分配单元/簇大小）对大文件性能的影响

---

## 阶段 6：实战项目（按优先级）

### 项目 1：简易日志系统 ⭐ 入门必做
- 上电创建 `log/2026-09-16.txt`，追加写入；文件超过 1MB 自动切割
- 练到：目录、追加、f_sync、字符串格式化

### 项目 2：SD 卡配置文件读写 ⭐ 
- 从 SD 卡读 `config.ini`（IP、采样率等参数），程序按配置运行；支持改写保存
- 练到：f_gets 逐行解析、字符串处理

### 项目 3：图片/BMP 文件生成与显示（探索者板有 LCD）
- 程序生成一张 BMP 写入 SD 卡；再读取 SD 卡里的图片解码显示到 LCD
- 练到：二进制文件、BMP 头结构、大文件 f_lseek 定位

### 项目 4：SD 卡 IAP 升级 ⭐⭐ 强烈推荐（衔接你的 A/B 区 bootloader）
- 你已有：A 区接收升级指令跳 B 区、B 区接收 bin 升级、24C02 存标志
- 现在加一条链路：**bin 文件放 SD 卡 → 上电检测 → 校验（CRC/长度）→ 搬运到 APP 区 → 更新 24C02 标志 → 跳转**
- 练到：大文件读取、hex/bin 解析、Flash 编程、与已有升级框架整合 —— 简历级项目

### 项目 5：USB 大容量存储（读卡器模式）
- F407 USB OTG + SD 卡，插电脑当 U 盘，电脑与 MCU 共享文件
- 练到：USB 协议栈、与 FatFs 共存时的互斥访问

### 项目 6：FreeRTOS + FatFs 多任务
- 一个任务写日志、一个任务读配置、一个任务删旧文件
- 练到：`FF_FS_REENTRANT`（R0.15 已改为由应用提供 `ff_mutex_take/give`）、互斥保护、优先级反转

---

## 阶段 7：原理深入（可选加分项）

- [ ] 通读 `ff.c` 源码：挂载流程、FAT 链遍历、目录项查找算法
- [ ] 用 WinHex 对照一个真实格式化的卡，手工解析：MBR → DBR → FAT → 根目录项 → 数据区
- [ ] 进阶挑战：**手写一个 mini-FAT16 只读解析器**（不依赖 FatFs，直接读扇区列出文件）
- [ ] 了解工程话题：掉电保护（事务日志/双缓冲）、FAT 的缺陷与 exFAT、磨损均衡（SD 卡内部已做）
- [ ] 了解 ChaN 的 `f_mkfs` 参数与 GPT、TRIM 的关系

---

## 学习资源

| 资源 | 说明 |
|------|------|
| [FatFs 官网](http://elm-chan.org/fsw/ff/) | 源码 + API 手册 + 配置说明，**第一手资料**，必收藏 |
| 正点原子《STM32F407 FreeRTOS 开发指南》FatFs 章节 | 与探索者板完全对应，跟做移植实验 |
| 野火《STM32 库开发实战指南》SDIO + FatFs 章节 | 另一家参考，交叉验证 |
| ffconf.h 中文注释版（网上搜） | 逐项解释每个宏 |
| WinHex | 查看卡内原始扇区，理解文件系统的神器 |
| ChaN 博客（英文原文） | 设计思想，阶段 7 再看 |

---

## 避坑清单（血泪经验浓缩）

1. **`get_fattime` 必须返回合法值**，否则文件时间乱甚至 `f_mkfs`/挂载异常
2. **新卡第一次用先 `f_mkfs`**，没格式化就挂载会返回 `FR_NO_FILESYSTEM`
3. `f_write` 返回 `FR_OK` 不代表写成功，**还要检查 `bw` 是否等于预期字节数**
4. 写完大文件**必须 `f_close` 或周期 `f_sync`**，否则缓存没落盘，掉电全丢
5. 多扇区读写接口要真支持多扇区，别在 diskio 里拆成单扇区循环（性能杀手）
6. `FF_USE_LFN = 1`（栈缓冲）时注意任务栈大小，栈溢出症状很诡异
7. 路径格式必须带盘符：`"0:log/test.txt"`，不能写 `/log/test.txt`（取决于 FF_VOLUME_STRS 设置）
8. SD 卡热插拔后要重新 `f_mount`，别指望自动恢复
9. DMA 读写时缓冲区地址/对齐问题会导致偶发 HardFault，优先用静态分配的缓冲
10. 调试时**先打返回码**：`printf("err=%d @line %d\n", fr, __LINE__)`，FR 码对照手册查，比猜快十倍

---

## 里程碑自检

- [x] 能用裸机命令写扇区并读回校验
- [ ] FatFs 挂载成功，f_mkfs + f_open + f_write 后电脑能打开文件
- [ ] 能用中文名建文件、能遍历目录打印文件树
- [ ] 顺序写速度 ≥ 1MB/s
- [ ] 完成 SD 卡 IAP：把 SD 卡里的 bin 烧进 APP 区并成功运行
- [ ] （选）USB 读卡器模式跑通

# DSH Web 一键启动器

桌面快捷方式 **DSH Web** 指向本目录的 `start-dsh-web.vbs`。

## 双击后会发生什么

1. 如果 3080 端口上已经有 DSH Web 服务在跑 → **只打开浏览器**，不会重复启一个实例。
2. 否则 → 无窗口静默启动 `dsh web`，等它就绪后用默认浏览器打开 Web UI。
3. 启动器作为服务的父进程常驻，**结束它就会结束服务**；注销或重启当然也会。

打开浏览器用的是服务自己打印的带令牌地址
（`http://127.0.0.1:3080/?token=...`）。这个令牌是唯一的进入方式：裸地址
`http://127.0.0.1:3080/` 会返回 401，所以不要手动敲地址、也不要把裸地址存成书签。

## 文件

| 文件 | 作用 |
|---|---|
| `start-dsh-web.vbs` | 无窗口外壳，快捷方式的目标 |
| `start-dsh-web.ps1` | 真正的启动逻辑 |
| `logs\launcher.log` | 启动器自己的记录（每次运行都会追加） |
| `logs\dsh-web.out.log` | 服务的标准输出，带令牌的 URL 在这里 |
| `logs\dsh-web.err.log` | 服务的错误输出，启动失败看这里 |
| `logs\state-<端口>.json` | 该端口最近一次成功的 URL 与进程号 |
| `logs\service-<端口>.pid` | 本启动器为该端口启动的服务进程号 |

## 手动用法

```powershell
# 与双击等价
powershell -NoProfile -File start-dsh-web.ps1

# 换端口 + 不打开浏览器（排查用）
powershell -NoProfile -File start-dsh-web.ps1 -Port 3099 -NoBrowser

# 也可以直接给 VBS 传参
wscript start-dsh-web.vbs 3099 nobrowser
```

## 改默认设置

直接编辑 `start-dsh-web.ps1` 顶部的 `param` 块：

- `$Workspace`：新会话的默认 workspace 根目录，当前是 `E:\MCU_TEST_CODE`。
- `$Port`：默认端口，当前是 3080。只需改这一处；`start-dsh-web.vbs`
  不硬编码端口，只在收到参数时才覆盖。
- `$WaitSeconds`：等待服务就绪的上限秒数，当前 90。

## 停止服务

- 任务管理器里结束该 `node.exe`，启动器会随之退出；或者
- `Get-Process node | Where-Object { $_.Path -like '*nodejs*' } | Stop-Process`（会杀掉所有 node，慎用）；或者
- 直接注销/重启。

## 排查

- 双击后浏览器没反应：看 `logs\launcher.log` 最后几行，再看 `logs\dsh-web.err.log`。
- 提示 401：说明打开的是裸地址，请用 `launcher.log` / `state-*.json` 里带 `?token=` 的完整地址。
- 重复启动被拦截：`launcher.log` 会写明是"端口已在服务"还是"pid 记录仍是该服务"。

# 19 如何在 Linux 上运行

本仓库的产品是 **MCU 固件**。Linux 目录是同一套 `USER/App` 的 PC 移植，用来脱离单片机调界面和框架。

有两套入口，**桌面电脑用 SDL2**；framebuffer 给带 `/dev/fb0` 的嵌入式板或虚拟终端用。

| | LinuxSDL2（推荐） | Linux（framebuffer） |
|--|-------------------|----------------------|
| 路径 | `Software/X-Track/LinuxSDL2` | `Software/X-Track/Linux` |
| 显示 | 弹出 480×320 窗口 | 画到 `/dev/fb0`，要切到 tty（Ctrl+Alt+F1～F6） |
| 输入 | 鼠标、键盘、滚轮 | `evdev`：鼠标/触摸节点 |
| 依赖 | `libsdl2-dev` | 一般只需 gcc，能访问 framebuffer |
| 启动 | `./xtrack` | `sudo ./xtrack /dev/input/eventX` |

官方说明：`LinuxSDL2/README.md`、`Linux/README.md`。  
共通限制：**关机自动保存不支持；轨迹记录未测，可能不可用。** GPS 在模拟 HAL 里用 GPX 回放，不是真模块。

---

## 1. 推荐：SDL2 窗口

### 1.1 装依赖（Debian / Ubuntu）

```sh
sudo apt update
sudo apt install -y build-essential libsdl2-dev
```

需要图形界面（本机桌面或 X11/Wayland 转发）。纯 SSH 且没有 `DISPLAY` 时窗口出不来。

### 1.2 编译

必须在该目录下执行（Makefile 用 `pwd` 找 `USER/App` 和模拟器 HAL）：

```sh
cd Software/X-Track/LinuxSDL2
make -j$(nproc)
```

产物：当前目录的 `xtrack`。清理：`make clean`。

编译用的 App 就是 `USER/App`（PageManager、DataCenter、表盘都在这里）；显示/输入/假 GPS 在 `Simulator/LVGL.Simulator/HAL`。

### 1.3 运行

**工作目录必须是 `LinuxSDL2/`**，不要把二进制拷走再跑。  
Makefile 里 `LV_FS_PC_PATH="../../../"`：相对本目录上三级，即仓库根 `X-TRACK/`。LVGL 打开 `/SystemSave.json`、`/MAP`、`/TRK_EXAMPLE.gpx` 都会拼到根目录下。

```sh
cd Software/X-Track/LinuxSDL2
./xtrack
```

应弹出窗口，先看到开机 Logo，约 2 秒后进表盘。

### 1.4 操作（代替旋转编码器）

`main.cpp` 注册了三种输入：

| 设备 | LVGL 类型 | 作用 |
|------|-----------|------|
| 鼠标 | POINTER | 点按钮（地图 / 录轨 / 菜单） |
| 滚轮 | ENCODER | 换焦点、拧缩放条（地图页） |
| 键盘 | KEYPAD | 进默认 group，可当编码器辅助 |

表盘：点地图进 LiveMap，点菜单进系统信息；录轨仍是长按/短按逻辑。  
地图页：滚轮改缩放；按缩放条或运动信息区返回。

### 1.5 数据文件（可选）

都放在 **仓库根目录**（和 `Software/` 同级），不是放在 `LinuxSDL2/` 里。

| 文件 | 作用 | 没有时 |
|------|------|--------|
| `SystemSave.json` | 里程、时区、地图路径等 | 用代码里的默认值；Linux 关机保存本就不保证 |
| `TRK_EXAMPLE.gpx` | 模拟 HAL 回放轨迹当「GPS」 | 用 Config 里默认经纬度，卫星/速度可能一直是空的 |
| `MAP/z/x/y.png` | 离线瓦片 | 地图页空白；Linux 默认扩展名是 **png**（真机 SD 卡常是 bin） |

地图怎么下：仓库 `Tools/README.md`。Linux 移植把 `CONFIG_MAP_EXT_NAME_DEFAULT` 设成了 `"png"`，瓦片保持 png 即可，不必转成 LVGL bin。目录仍叫 `MAP`，层级 `z/x/y.png`。

---

## 2. 备选：Framebuffer + evdev

适合没有窗口系统、直接操作 framebuffer 的板子。普通 Ubuntu 桌面不建议走这条：要切虚拟终端，还容易和图形会话抢屏幕。

```sh
sudo apt install -y build-essential
cd Software/X-Track/Linux
make -j$(nproc)
```

查输入节点：

```sh
sudo apt install -y evtest
sudo evtest          # 看哪个 event 是鼠标/触摸
```

运行：

```sh
sudo ./xtrack /dev/input/event0
```

然后 `Ctrl+Alt+F1`～`F6` 切到 framebuffer 终端才能看见 UI。  
`event0` 按你机器改。分辨率跟 framebuffer 实际宽高，不是写死 240×240。

同样必须在 `Linux/` 目录启动，文件路径规则与 SDL2 相同（也是 `../../../`）。

---

## 3. 和真机差在哪（心里有数）

```
LinuxSDL2/main.cpp
    lv_init / SDL 窗口 / 鼠标键盘
    HAL::HAL_Init()     → Simulator/LVGL.Simulator/HAL（假 GPS、假编码器）
    App_Init()          → 与真机相同的 USER/App
    循环：lv_task_handler + HAL_Update
```

真机是 `USER/main.cpp` + `USER/HAL` + ST7789 + 真编码器。  
所以：在 Linux 上学 PageManager / DataCenter / MVP 足够；测屏幕时序、SD 卡、真实 GPS 必须上板。

模拟 HAL 要点：

- `HAL_GPS.cpp`：读 `/TRK_EXAMPLE.gpx` 推进经纬度
- `HAL_Encoder.cpp`：`GetDiff` 恒为 0，旋钮事件走的是 LVGL 的 SDL 滚轮，不是这套 HAL
- 蜂鸣器/音频在 PC 上往往无声或很弱

Makefile 还可改 `LV_COLOR_DEPTH`（默认 32）。SDL2 窗口分辨率是 `LV_HOR_RES=480`、`LV_VER_RES=320`，比真机 240×240 大，布局会拉伸感，属正常。

---

## 4. 常见问题

| 现象 | 处理 |
|------|------|
| `fatal error: SDL2/SDL.h` | `sudo apt install libsdl2-dev` |
| 编译过了没窗口 | 检查 `echo $DISPLAY`；SSH 需 X11 转发或在桌面本机跑 |
| 能进表盘但没速度 | 根目录放 `TRK_EXAMPLE.gpx`，或接受默认点 |
| 地图全黑 | 根目录放 `MAP/`，png 瓦片；看 `SystemSave.json` 的 `mapDirPath` |
| 点了没反应 | SDL2 用鼠标点；framebuffer 换对 `event` 节点 |
| 从别的目录启动找不到 json/地图 | 回到 `LinuxSDL2/` 再 `./xtrack` |

---

## 5. 最短命令清单（桌面）

```sh
sudo apt install -y build-essential libsdl2-dev
cd /home/gengtao/work_xtrack/X-TRACK/Software/X-Track/LinuxSDL2
make -j$(nproc)
./xtrack
```

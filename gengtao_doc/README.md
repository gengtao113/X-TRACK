# X-TRACK 学习笔记

> 编码：UTF-8（带 BOM）。本目录 `.editorconfig` 已指定 `charset = utf-8`。


本目录是对当前仓库的框架梳理，方便后续阅读源码。
**原工程是单片机项目**（AT32F435 / AT32F403A + LVGL v8），Windows 模拟器和 Linux 移植是同一套 App 的额外运行时。

固件版本：`v2.7`（见 `Software/X-Track/USER/App/Version.h`）

## 文档索引

| 文档 | 内容 |
|------|------|
| [01-项目定位.md](01-项目定位.md) | 这是什么产品、Linux 还是 MCU、仓库目录 |
| [02-软件架构.md](02-软件架构.md) | 六层结构、启动链路、HAL、调度 |
| [03-数据流.md](03-数据流.md) | DataCenter 发布订阅、13 个 DataProc 节点 |
| [04-页面框架.md](04-页面框架.md) | MVP、PageManager 生命周期、如何加页面 |
| [05-学习路径.md](05-学习路径.md) | 最值得学的点、推荐阅读顺序 |
| [06-页面框架-C语言说明.md](06-页面框架-C语言说明.md) | 用 C（结构体/函数指针/状态机）解释页面框架好在哪 |
| [07-数据流-C语言说明.md](07-数据流-C语言说明.md) | 用 C（邮箱/回调/乒乓缓存）解释 DataCenter 怎么传数据 |
| [08-CmBacktrace栈回溯与STM32移植.md](08-CmBacktrace栈回溯与STM32移植.md) | HardFault 栈回溯原理，以及搬到 STM32 的步骤 |
| [09-总线四句话-对照源码.md](09-总线四句话-对照源码.md) | Publish/Pull/Notify/Timer 对照源码的 C 写法 |
| [10-Dialplate-View-C语言.md](10-Dialplate-View-C语言.md) | 表盘 View：LVGL 控件如何创建 |
| [11-Dialplate-Model-C语言.md](11-Dialplate-Model-C语言.md) | 表盘 Model：总线账号与缓存 |
| [12-Dialplate-Presenter-C语言.md](12-Dialplate-Presenter-C语言.md) | 表盘 Presenter：生命周期与录轨状态机 |
| [13-裸机框架-C语言.md](13-裸机框架-C语言.md) | 无 RTOS：main 循环、MillisTask、LVGL timer、10ms 中断 |
| [14-PageManager-C语言.md](14-PageManager-C语言.md) | 页面池/栈、Install/Push/Pop/Replace、生命周期状态机 |
| [15-六个文件夹的业务.md](15-六个文件夹的业务.md) | Dialplate / LiveMap / StartUp / StatusBar / SystemInfos / _Template |

## 源码入口（优先打开）

```
Software/X-Track/USER/main.cpp                          启动与主循环
Software/X-Track/USER/App/App.cpp                       应用初始化、装页面
Software/X-Track/USER/HAL/HAL.cpp                       MCU 侧 HAL 任务表
Software/X-Track/USER/App/Common/HAL/HAL.h              HAL 接口（无实现）
Software/X-Track/USER/App/Common/DataProc/DP_LIST.inc   业务节点注册
Software/X-Track/USER/App/Pages/_Template/              新页面模板
Software/X-Track/USER/App/Pages/Dialplate/              主表盘（MVP 样板）
Software/X-Track/USER/App/Utils/PageManager/            页面调度器
Software/X-Track/USER/App/Utils/DataCenter/             发布订阅总线
```

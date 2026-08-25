# 08 CmBacktrace 栈回溯与 STM32 移植

库位置：`Software/X-Track/Libraries/cm_backtrace`  
作者 Armink，版本见 `cmb_def.h` 中 `CMB_SW_VERSION`（本仓库为 1.4.0）。  
本工程接入点：`USER/HAL/HAL_FaultHandle.cpp`，启动时 `HAL::HAL_Init()` 会调 `FaultHandle_Init()`。

它是 **纯 C + 一小段汇编** 的 Cortex-M 通用库，和 AT32 / STM32 品牌无关。STM32 移植主要是改配置、链接符号和 `HardFault_Handler`，不要改回溯算法。

---

## 1. 它解决什么问题

Cortex-M 一进 `HardFault_Handler`，现场已经切到 Handler 模式。默认弱符号往往是：

```c
void HardFault_Handler(void)
{
    while (1) {}
}
```

你只知道「死了」，不知道：

- 死在哪条指令（PC）
- 从哪个函数调过来（LR / 调用栈）
- 是空指针、除零、还是栈溢出（fault 状态寄存器）

CmBacktrace 在 HardFault 里做三件事：

1. 从异常自动压栈帧里取出 R0~R3、R12、LR、PC、xPSR  
2. 读 SCB 的 HFSR / MFSR / BFSR / UFSR，用中文/英文打出原因  
3. 沿栈扫描，找出像「函数返回地址」的数，拼成调用栈，串口打出一串地址

再配合 PC 上的 `addr2line`，地址就能对上 `.c` 的行号。

本工程额外做了：串口打印之后，屏幕显示 `"FXXK HardFault!"`，等按编码器再复位（见 `vApplicationHardFaultHook`）。

---

## 2. 目录里各文件干什么

```
cm_backtrace/
├── cm_backtrace.h / .c     回溯主体（C）
├── cmb_def.h               CPU 类型、SCB 寄存器地址、栈/代码段符号名
├── cmb_cfg.h               本工程配置（必须改的只有这一份）
├── Languages/              打印文案
└── fault_handler/
    ├── keil/cmb_fault.S    Keil 的 HardFault_Handler
    ├── gcc/cmb_fault.S     GCC 的 HardFault_Handler
    └── iar/cmb_fault.S
```

X-TRACK **没有用** `fault_handler/keil/cmb_fault.S`，而是在 `HAL_FaultHandle.cpp` 里自己写了一个同功能的 `HardFault_Handler`（打印完还要刷屏、等待按键复位）。两边不要同时提供这个符号，否则链接会重复定义。

启动文件里的 `HardFault_Handler` 是 `[WEAK]`，你的强符号会覆盖它。这是对的。

---

## 3. 异常时硬件已经帮你压了什么

进入 HardFault 时，CPU **自动** 把当时的寄存器压到当时的栈上（裸机一般是 MSP）：

```
高地址
  xPSR
  PC      <- 出问题的那条指令（或下一条）
  LR      <- 返回地址
  R12
  R3
  R2
  R1
  R0
低地址  <- 进入 Handler 后的 SP 指向这里
```

若开了 FPU，且 LR 的 bit4=0，后面还会多 S0~S15、FPSCR（对齐到 8 字节）。  
`cm_backtrace_fault(lr, sp)` 的两个参数就是 Handler 入口处的 LR 和 SP。本工程写法：

```c
/* HAL_FaultHandle.cpp，Keil 内联汇编。C 里等价如下 */
void HardFault_Handler(void)
{
    /* r0 = lr, r1 = sp */
    cm_backtrace_fault( /*lr*/ , /*sp*/ );
    vApplicationHardFaultHook();
    while (1) {}
}
```

对应汇编：

```
MOV  r0, lr
MOV  r1, sp
BL   cm_backtrace_fault
```

EXC_RETURN（此时的 LR）bit2：0=出错时用 MSP，1=用 PSP（RTOS 线程）。  
裸机只需 MSP；STM32 + FreeRTOS 时库会读 PSP。

---

## 4. 调用栈是怎么扫出来的（核心思路）

Cortex-M 用 Thumb，指令地址最低位为 1。函数调用是 `BL` / `BLX`，返回地址（LR）会留在栈上。

`cm_backtrace_call_stack()` 做的事用 C 描述：

```c
/* 伪代码，对应 cm_backtrace.c */
size_t walk_stack(uint32_t *out, size_t max, uint32_t sp)
{
    size_t n = 0;

    /* 1. 先放入异常帧里的 PC、LR（最准） */
    out[n++] = saved_pc;
    out[n++] = saved_lr | 1;   /* Thumb 地址 */

    /* 2. 从 sp 往栈顶扫每一个字 */
    for (; sp < stack_end; sp += 4) {
        uint32_t cand = *(uint32_t *)sp;
        if ((cand & 1) == 0)
            continue;                         /* 不是 Thumb 地址 */
        cand -= 1;
        if (cand < code_start || cand > code_end)
            continue;                         /* 不在 Flash 代码段 */
        if (!prev_insn_is_bl_or_blx(cand))
            continue;                         /* 前一条不是 BL/BLX，多半是数据 */
        out[n++] = cand;
        if (n >= max) break;
    }
    return n;
}
```

所以它 **不是** DWARF 调试器那种完整 backtrace，而是「在栈上捞像返回地址的数」。  
优化开太高、尾调用、没保存 LR 的叶子函数，栈上会缺帧，这是正常的。

代码段、主栈范围在 `cm_backtrace_init()` 里从链接器符号读出：

| 编译器 | 栈起止（默认） | 代码起止（默认） |
|--------|----------------|------------------|
| Keil | `STACK$$Base` / `STACK$$Limit` | `Image$$ER_IROM1$$Base` / `Limit` |
| GCC | `_sstack` / `_estack` | `_stext` / `_etext` |
| IAR | `"CSTACK"` | `".text"` |

本工程 Keil scatter 里加载区就叫 `ER_IROM1`，和默认宏一致，所以 `cmb_cfg.h` 没改段名。

---

## 5. 本工程是怎么接上的

`cmb_cfg.h`：

- `CMB_USING_BARE_METAL_PLATFORM`：无 RTOS  
- `CMB_CPU_PLATFORM_TYPE = CMB_CPU_ARM_CORTEX_M4`（AT32F435/F403A 都是 M4 类核）  
- `CMB_USING_DUMP_STACK_INFO`：把栈内容 hex dump 出来  
- `cmb_println` -> 自己实现的 `cmb_printf` -> `Serial.print`

初始化：

```c
cm_backtrace_init("X-TRACK", "v1.0", "v2.7 " __DATE__);
```

之后 HardFault 串口会打出固件名、寄存器、fault 原因、一串 PC。用同一份 `.axf` / `.elf` 做 addr2line：

```sh
arm-none-eabi-addr2line -e X-Track.axf -a -f 08001234 08004567
```

Keil 也可用 fromelf / 调试器反汇编窗口，把 PC 对到函数。

---

## 6. 移植到 STM32 的思路（按顺序做）

STM32 和 AT32 共用 Cortex-M 内核和同一套 SCB 地址（`0xE000EDxx`），**库源码可以原样拷贝**。

### 步骤 1：拷文件进工程

至少加入：

- `cm_backtrace.c`
- `cm_backtrace.h`、`cmb_def.h`、`cmb_cfg.h`
- `Languages/`（头文件被 .c include）
- 按工具链选一份 `fault_handler/.../cmb_fault.S`，**或者** 像本工程一样自己写 Handler

包含路径指到 `cm_backtrace/` 目录。语言选中文时，确认 `cmb_zh_CN.h` 存在；本仓库 `CMB_PRINT_LANGUAGE_CHINESE` 依赖的是 `Languages/zh-CN/cmb_zh_CN.h`（若缺文件，改成 `CMB_PRINT_LANGUAGE_CHINESE_UTF8` 并用 UTF8 那份）。

### 步骤 2：改 `cmb_cfg.h`

```c
#define cmb_println(...)    printf(__VA_ARGS__); printf("\r\n")

#define CMB_USING_BARE_METAL_PLATFORM     /* 裸机 */
/* #define CMB_USING_OS_PLATFORM */
/* #define CMB_OS_PLATFORM_TYPE  CMB_OS_PLATFORM_FREERTOS */

#if defined(STM32F0xx) || defined(STM32G0xx) || defined(STM32L0xx)
#define CMB_CPU_PLATFORM_TYPE   CMB_CPU_ARM_CORTEX_M0
#elif defined(STM32F1xx) || defined(STM32F3xx) || defined(STM32L1xx)
#define CMB_CPU_PLATFORM_TYPE   CMB_CPU_ARM_CORTEX_M3   /* 部分 F1 是 M3 */
#elif defined(STM32F4xx) || defined(STM32G4xx) || defined(STM32L4xx) || defined(STM32F3)
#define CMB_CPU_PLATFORM_TYPE   CMB_CPU_ARM_CORTEX_M4
#elif defined(STM32F7xx) || defined(STM32H7xx)
#define CMB_CPU_PLATFORM_TYPE   CMB_CPU_ARM_CORTEX_M7
#elif defined(STM32L5xx) || defined(STM32U5xx) || defined(STM32H5xx)
#define CMB_CPU_PLATFORM_TYPE   CMB_CPU_ARM_CORTEX_M33
#endif

#define CMB_USING_DUMP_STACK_INFO
#define CMB_PRINT_LANGUAGE      CMB_PRINT_LANGUAGE_CHINESE_UTF8
```

核选错（例如 F4 写成 M3）时，FPU 压栈长度会算错，调用栈会偏。  
F0/G0 是 M0：库 **不诊断** UFSR/BFSR 细项，仍能打印 PC/LR 和扫栈。

`printf` 必须在 HardFault 里还能用：UART 轮询发送，不要依赖中断、不要 malloc。本工程用 `Serial.print` + 栈上 256 字节缓冲，就是这个原因。

### 步骤 3：保证栈、代码段符号对得上

**Keil（STM32 官方 pack / MDK）**  
默认 `STACK` + `ER_IROM1` 一般能直接用。若 scatter 把 Flash 改成 `ER_IROM2` 之类，在 `cmb_cfg.h` 里：

```c
#define CMB_CODE_SECTION_NAME   ER_IROM2
```

**STM32CubeIDE / Makefile + GCC**（最容易踩坑）

Cube 的 `.ld` 通常只有 `_estack`（RAM 最高地址，栈向下长），**没有** `_sstack`、`_stext`、`_etext`。  
必须在链接脚本里补，或改宏去对已有符号，例如：

```ld
/* 在 STM32Fxxx_FLASH.ld 里 */
_estack = ORIGIN(RAM) + LENGTH(RAM);
_Min_Stack_Size = 0x400;              /* 与 Cube 的 _Min_Stack_Size 一致 */
_sstack = _estack - _Min_Stack_Size;

_stext = LOADADDR(.text);             /* 或直接 . = ORIGIN(FLASH); _stext = .; */
_etext = ADDR(.text) + SIZEOF(.text);
```

若你的脚本已有 `_sidata` / `_etext`，在 `cmb_cfg.h` 覆盖：

```c
#define CMB_CSTACK_BLOCK_START   _sstack
#define CMB_CSTACK_BLOCK_END     _estack
#define CMB_CODE_SECTION_START   _stext
#define CMB_CODE_SECTION_END     _etext
```

`cm_backtrace_init()` 里若打印「主栈配置错误」，就是这段长度为 0，符号没链上。

### 步骤 4：只保留一个 HardFault_Handler

Cube 会在 `stm32fxxx_it.c` 生成空的 `HardFault_Handler`。必须处理冲突，三选一：

1. 删掉 / 注释 `stm32xxxx_it.c` 里的 HardFault（以及可选的 MemManage/BusFault/UsageFault）  
2. 不编译库自带的 `cmb_fault.S`，在 `it.c` 里改成调 `cm_backtrace_fault`  
3. 把 `it.c` 里的函数改成弱符号，让汇编文件覆盖

推荐在 `stm32fxxx_it.c` 里写（裸机、无 FPU 额外处理时与库汇编等价）：

```c
#include "cm_backtrace.h"

void HardFault_Handler(void)
{
    __asm volatile (
        "mov r0, lr\n"
        "mov r1, sp\n"
        "bl cm_backtrace_fault\n"
    );
    while (1) {}
}
```

有 FPU 的 F4/F7，仍把 LR 原样传入即可，库内部会看 bit4 跳过 FPU 帧。

若用 FreeRTOS：

```c
#define CMB_USING_OS_PLATFORM
#define CMB_OS_PLATFORM_TYPE    CMB_OS_PLATFORM_FREERTOS
```

不要定义 `CMB_USING_BARE_METAL_PLATFORM`。线程里出错时 LR.bit2=1，库会改去读 PSP。

### 步骤 5：在 main 里初始化，越早越好

UART 初始化之后立刻：

```c
cm_backtrace_init("my_app", "hw1.0", "sw0.1");
```

固件名要和 PC 上 elf 文件名对得上，打印调用栈时会提示 `addr2line -e my_app.elf ...`。

可选：断言失败时也回溯：

```c
void assert_failed(uint8_t *file, uint32_t line)
{
    cm_backtrace_assert(cmb_get_sp());  /* cmb_get_sp 在 cmb_def.h 里内联汇编 */
    while (1) {}
}
```

`cmb_get_sp` / `cmb_get_msp` / `cmb_get_psp` 已按 ARMCC / GCC / IAR 写好，STM32 不用自己实现。

### 步骤 6：建议打开的内核开关（便于诊断）

在 `SystemInit` 或 main 里（M3/M4/M7）：

```c
SCB->CCR |= SCB_CCR_DIV_0_TRP_Msk;   /* 除零进 UsageFault，而不是静默 Inf */
SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk
           |  SCB_SHCSR_BUSFAULTENA_Msk
           |  SCB_SHCSR_MEMFAULTENA_Msk;
```

这样除零、非法指令会先到 UsageFault。若 UsageFault 的 Handler 仍是弱 while(1)，可以把 MemManage/BusFault/UsageFault **都跳到同一个** `cm_backtrace_fault`（参数同样传 lr、sp）。不使能时，很多错会合并成 HardFault，库照样能从 HFSR.FORCED 往下解析 UFSR。

### 步骤 7：在 PC 上还原行号

保留 **带符号** 的 elf（Cube 的 `.elf`，Keil 勾 Debug Information）。

```sh
arm-none-eabi-addr2line -e build/your_app.elf -a -f -p 08001234 08004567 08007890
```

Windows Keil：用同样地址在 Disassembly 窗口跳转，或：

```
fromelf --text -c your.axf
```

---

## 7. 对照检查清单

| 现象 | 先查 |
|------|------|
| 链接重复定义 HardFault_Handler | it.c 和 cmb_fault.S 同时存在 |
| init 报主栈配置错误 | GCC 缺 `_sstack`/`_estack`，或 Keil 段名不是 STACK |
| 调用栈全是垃圾地址 | 代码段符号不是真正的 `.text`；或 CPU 类型选错导致 SP 跳过的帧长度不对 |
| HardFault 里 printf 卡死 | UART 用了中断/DMA，改成轮询 |
| 有 RTOS 却扫到 MSP 垃圾 | 没开 `CMB_USING_OS_PLATFORM`，线程故障应走 PSP |
| F4 开 FPU 后栈解析乱 | `CMB_CPU_PLATFORM_TYPE` 必须是 M4/M7，不能当 M3 |

---

## 8. 和本仓库的对应关系

| STM32 上你要做的 | X-TRACK 里对应 |
|------------------|----------------|
| `cmb_cfg.h` | 已配裸机 + M4 + 中文 |
| `cmb_println` | `cmb_printf` -> `Serial.print` |
| `cm_backtrace_init` | `HAL::FaultHandle_Init()` |
| `HardFault_Handler` | `HAL_FaultHandle.cpp` 内联汇编 |
| 出错后复位 | `vApplicationHardFaultHook` 刷屏，松手编码器 `NVIC_SystemReset` |

读源码顺序：`cmb_cfg.h` -> `HAL_FaultHandle.cpp` -> `cm_backtrace_fault()` -> `cm_backtrace_call_stack()`。

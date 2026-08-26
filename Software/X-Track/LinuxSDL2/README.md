# X-TRACK Linux SDL2

## 编译

在仓库根目录：

```sh
./gengtao_build.sh
```

或在本目录用 CMake（目标文件在仓库根 `build_out_dir/`）：

```sh
cmake -S . -B ../../../build_out_dir
cmake --build ../../../build_out_dir -j
```

## 执行

工作目录必须是本目录（`LV_FS_PC_PATH=../../../` 相对这里指向仓库根）：

```sh
./xtrack
```

也可：`./gengtao_build.sh run`

## 其他说明

* 1.可以通过修改 `CMakeLists.txt` 的 `LV_COLOR_DEPTH` 配置屏幕的颜色深度，默认32bpp。
* 2.关机自动保存功能不支持。
* 3.轨迹记录功能未测试，可能不支持。

## Demo
![image](./media/demo_sdl.png)

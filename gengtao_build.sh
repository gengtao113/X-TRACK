#!/usr/bin/env bash
# 仓库根目录：用 CMake 编译 / 运行 / 清理 Linux SDL2 模拟器。
#   ./gengtao_build.sh        编译
#   ./gengtao_build.sh run    编译后运行（工作目录必须是 LinuxSDL2）
#   ./gengtao_build.sh clean  删除 build_out_dir、xtrack，以及源码旁残留的 .o
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
SRC_DIR="${ROOT}/Software/X-Track/LinuxSDL2"
OUT_DIR="${ROOT}/build_out_dir"
XTRACK_DIR="${ROOT}/Software/X-Track"
CMD="${1:-build}"

usage() {
    echo "usage: $0 [build|run|clean]"
    echo "  (default) build  cmake compile LinuxSDL2 -> xtrack"
    echo "  run              compile if needed, then ./xtrack"
    echo "  clean            remove build_out_dir, xtrack, and leftover *.o"
}

if [[ ! -f "${SRC_DIR}/CMakeLists.txt" ]]; then
    echo "error: CMakeLists.txt not found: ${SRC_DIR}/CMakeLists.txt" >&2
    exit 1
fi

cmake_build() {
    echo "src dir: ${SRC_DIR}"
    echo "obj dir: ${OUT_DIR}"
    cmake -S "${SRC_DIR}" -B "${OUT_DIR}"
    cmake --build "${OUT_DIR}" --parallel "$(nproc)"
}

case "${CMD}" in
    build|"")
        cmake_build
        echo "ok: ${SRC_DIR}/xtrack"
        echo "run:  $0 run"
        ;;
    run)
        cmake_build
        echo "run: ${SRC_DIR}/xtrack"
        cd "${SRC_DIR}"
        exec ./xtrack
        ;;
    clean)
        echo "clean: ${OUT_DIR}"
        rm -rf "${OUT_DIR}"
        rm -f "${SRC_DIR}/xtrack"
        # 旧 Makefile 把 .o 写在 .c/.cpp 旁边；CMake 只往 build_out_dir 写，这些残留要单独扫
        leftover="$(find "${XTRACK_DIR}" -type f -name '*.o' | wc -l)"
        find "${XTRACK_DIR}" -type f -name '*.o' -delete
        echo "clean leftover in-source .o: ${leftover}"
        echo "ok: cleaned"
        ;;
    -h|--help|help)
        usage
        ;;
    *)
        echo "error: unknown command: ${CMD}" >&2
        usage >&2
        exit 1
        ;;
esac

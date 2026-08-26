#!/usr/bin/env bash
# 仓库根目录：编译 / 运行 / 清理 Linux SDL2 模拟器。
#   ./gengtao_build.sh        编译
#   ./gengtao_build.sh run    编译后运行（工作目录必须是 LinuxSDL2）
#   ./gengtao_build.sh clean  删除目标文件和 xtrack
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${ROOT}/Software/X-Track/LinuxSDL2"
CMD="${1:-build}"

usage() {
    echo "usage: $0 [build|run|clean]"
    echo "  (default) build  compile LinuxSDL2 -> xtrack"
    echo "  run              compile if needed, then ./xtrack"
    echo "  clean            make clean (remove .o and xtrack)"
}

if [[ ! -f "${BUILD_DIR}/Makefile" ]]; then
    echo "error: Makefile not found: ${BUILD_DIR}/Makefile" >&2
    exit 1
fi

cd "${BUILD_DIR}"

case "${CMD}" in
    build|"")
        echo "build dir: ${BUILD_DIR}"
        make -j"$(nproc)"
        echo "ok: ${BUILD_DIR}/xtrack"
        echo "run:  $0 run"
        ;;
    run)
        echo "build dir: ${BUILD_DIR}"
        make -j"$(nproc)"
        echo "run: ${BUILD_DIR}/xtrack"
        exec ./xtrack
        ;;
    clean)
        echo "clean dir: ${BUILD_DIR}"
        make clean
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

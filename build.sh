#
#!/bin/bash
# Infer-Frame 构建脚本
# 用法: ./build.sh [platform] [build_type]
#   platform: x86_64_cuda | aarch64_jetson | aarch64_rknn | aarch64_sophon
#   build_type: Debug | Release (default: Release)

set -e  # 遇到错误立即退出

# 项目根目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 引用颜色脚本
if [ -f "$SCRIPT_DIR/scripts/color.sh" ]; then
    source "$SCRIPT_DIR/scripts/color.sh"
fi

# 默认参数
PLATFORM=""
DETECTED_PLATFORM=""
BUILD_TYPE="Release"
BUILD_DIR="build"
CLEAN_BUILD=0
INSTALL_PREFIX="/usr/local"

# 平台检测函数
# 检测结果设置到全局变量 DETECTED_PLATFORM
detect_platform() {
    # 检测昇腾 (Ascend)
    if [ -d "/usr/local/Ascend" ] || [ -d "/usr/local/ascend" ]; then
        DETECTED_PLATFORM="aarch64_ascend"
        print_info "Detected Ascend platform: /usr/local/Ascend"
        return 0
    fi
    
    # 检测算能 (Sophon)
    if [ -d "/opt/sophon" ]; then
        DETECTED_PLATFORM="aarch64_sophon"
        print_info "Detected Sophon platform: /opt/sophon"
        return 0
    fi
    
    # 检测瑞芯微 (Rockchip)
    if [ -d "/usr/lib/aarch64-linux-gnu/librockchip_mpp.so" ] || \
       [ -d "/usr/lib/librknnrt.so" ] || \
       [ -f "/usr/lib/aarch64-linux-gnu/librknnrt.so" ]; then
        DETECTED_PLATFORM="aarch64_rknn"
        print_info "Detected Rockchip platform: RKNN runtime found"
        return 0
    fi
    
    # 检测 NVIDIA (CUDA/TensorRT)
    if command -v nvcc &> /dev/null || \
       [ -d "/usr/local/cuda" ] || \
       [ -f "/usr/lib/x86_64-linux-gnu/libnvinfer.so" ] || \
       [ -f "/usr/lib/aarch64-linux-gnu/libnvinfer.so" ]; then
        
        # 判断是 Jetson 还是 x86_64
        if [ -f "/etc/nv_tegra_release" ] || [ -d "/usr/lib/aarch64-linux-gnu/tegra" ]; then
            DETECTED_PLATFORM="aarch64_jetson"
            print_info "Detected NVIDIA Jetson platform"
        else
            DETECTED_PLATFORM="x86_64_cuda"
            print_info "Detected x86_64 CUDA platform"
        fi
        return 0
    fi
    
    # 默认平台
    local arch=$(uname -m)
    if [ "$arch" = "x86_64" ]; then
        DETECTED_PLATFORM="x86_64"
        print_warn "No specific platform detected, using generic x86_64"
    elif [ "$arch" = "aarch64" ]; then
        DETECTED_PLATFORM="aarch64"
        print_warn "No specific platform detected, using generic aarch64"
    else
        print_error "Unsupported architecture: $arch"
        return 1
    fi
    
    return 0
}

# 显示用法
show_usage() {
    cat << EOF
Usage: $0 [OPTIONS]

Options:
  --platform PLATFORM    Target platform (auto|x86_64_cuda|aarch64_jetson|aarch64_rknn|aarch64_sophon|aarch64_ascend)
                         Default: auto (auto-detect platform)
  --type TYPE            Build type (Debug|Release), default: Release
  --clean                Clean build directory before building
  --install-prefix PATH  Install prefix, default: /usr/local
  --build-algorithm      Build algorithms
  --build-tests          Build unit tests
  -h, --help             Show this help message

Platform Detection & Backend Mapping:
  The script automatically detects the platform and enables the corresponding inference backend:
  - Ascend (昇腾):  /usr/local/Ascend exists     → AscendCL backend
  - Sophon (算能):  /opt/sophon exists           → Sophon backend
  - RKNN (瑞芯微):  librknnrt.so found           → RKNN backend
  - Jetson (英伟达): Tegra platform + CUDA       → TensorRT backend
  - x86_64 CUDA:    CUDA on x86_64               → TensorRT backend

Examples:
  $0                                      # Auto-detect platform and enable backend
  $0 --platform aarch64_jetson            # Force Jetson platform (TensorRT)
  $0 --platform aarch64_ascend            # Force Ascend platform (AscendCL)
  $0 --platform x86_64_cuda --type Debug  # x86_64 CUDA Debug build
  $0 --clean --build-algorithm            # Clean build with algorithms

EOF
}

# 解析命令行参数
BUILD_PLUGINS=0
BUILD_TESTS=0

while [[ $# -gt 0 ]]; do
    case $1 in
        --platform)
            PLATFORM="$2"
            shift 2
            ;;
        --type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        --clean)
            CLEAN_BUILD=1
            shift
            ;;
        --install-prefix)
            INSTALL_PREFIX="$2"
            shift 2
            ;;
        --build-algorithm)
            BUILD_PLUGINS=1
            shift
            ;;
        --build-tests)
            BUILD_TESTS=1
            shift
            ;;
        -h|--help)
            show_usage
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done

# 切换到项目根目录
cd "$SCRIPT_DIR"

print_info "======================================"
print_info "  Infer-Frame Build Script"
print_info "======================================"

# 平台检测与设置
if [ -z "$PLATFORM" ] || [ "$PLATFORM" = "auto" ]; then
    print_info "Auto-detecting platform..."
    detect_platform
    if [ $? -ne 0 ]; then
        print_error "Failed to detect platform"
        exit 1
    fi
    PLATFORM="$DETECTED_PLATFORM"
fi

print_info "Target platform: $PLATFORM"
print_info "Build type: $BUILD_TYPE"

# 检查子模块
if [ ! -f "3rdparty/nndeploy/CMakeLists.txt" ]; then
    print_warn "NNDeploy submodule not initialized"
    print_info "Initializing submodules..."
    git submodule update --init --recursive
fi

# 清理构建目录
if [ $CLEAN_BUILD -eq 1 ]; then
    print_info "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
    # 同时清理 nndeploy 源码目录下的demo build
    rm -rf "3rdparty/nndeploy/build"
fi

# 创建构建目录
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# 根据平台拷贝对应的 NNDeploy config.cmake
print_info "Copying NNDeploy config.cmake for platform: $PLATFORM"

CONFIG_SOURCE=""
case $PLATFORM in
    aarch64_jetson|x86_64_cuda)
        CONFIG_SOURCE="$SCRIPT_DIR/cmake/nndeploy_jetson_config.cmake"
        ;;
    aarch64_rknn)
        CONFIG_SOURCE="$SCRIPT_DIR/cmake/nndeploy_rknn_config.cmake"
        ;;
    aarch64_ascend)
        CONFIG_SOURCE="$SCRIPT_DIR/cmake/nndeploy_ascend_config.cmake"
        ;;
    aarch64_sophon)
        CONFIG_SOURCE="$SCRIPT_DIR/cmake/nndeploy_sophon_config.cmake"
        ;;
    *)
        # 使用 Jetson 配置作为默认
        CONFIG_SOURCE="$SCRIPT_DIR/cmake/nndeploy_jetson_config.cmake"
        print_warn "Unknown platform, using Jetson config as default"
        ;;
esac

# 创建 NNDeploy build 目录并拷贝 config.cmake
mkdir -p 3rdparty/nndeploy
if [ -f "$CONFIG_SOURCE" ]; then
    cp "$CONFIG_SOURCE" 3rdparty/nndeploy/config.cmake
    print_info "Copied config from: $CONFIG_SOURCE"
else
    print_error "Config file not found: $CONFIG_SOURCE"
    exit 1
fi


# 确定工具链文件
TOOLCHAIN_FILE=""
# 只有在真正需要交叉编译时才使用工具链文件
# Jetson 平台本地编译不需要工具链文件
if [ -n "$PLATFORM" ] && [ "$PLATFORM" != "auto" ]; then
    case $PLATFORM in
        aarch64_jetson|x86_64_cuda|x86_64|aarch64)
            # 本地编译平台，不需要工具链文件
            print_info "Native build for $PLATFORM, no toolchain needed"
            ;;
        *)
            # 其他平台，尝试查找工具链文件
            TOOLCHAIN_FILE="$SCRIPT_DIR/platforms/${PLATFORM}.cmake"
            if [ ! -f "$TOOLCHAIN_FILE" ]; then
                print_warn "Toolchain file not found: $TOOLCHAIN_FILE"
                print_info "Proceeding without platform-specific toolchain..."
                TOOLCHAIN_FILE=""
            else
                print_info "Using toolchain: $TOOLCHAIN_FILE"
            fi
            ;;
    esac
fi

# 构建 CMake 命令
CMAKE_ARGS=(
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX"
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
)

# CUDA 平台需要设置 CUDA 编译器路径
if [[ "$PLATFORM" == *"cuda"* ]] || [[ "$PLATFORM" == *"jetson"* ]]; then
    if [ -f "/usr/local/cuda/bin/nvcc" ]; then
        CMAKE_ARGS+=(-DCMAKE_CUDA_COMPILER="/usr/local/cuda/bin/nvcc")
        print_info "Setting CUDA compiler: /usr/local/cuda/bin/nvcc"
    fi
fi

if [ -n "$TOOLCHAIN_FILE" ]; then
    CMAKE_ARGS+=(-DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE")
fi

if [ $BUILD_PLUGINS -eq 1 ]; then
    CMAKE_ARGS+=(-DBUILD_PLUGINS=ON)
fi

if [ $BUILD_TESTS -eq 1 ]; then
    CMAKE_ARGS+=(-DBUILD_TESTS=ON)
fi

# 配置
print_info "Configuring with CMake..."
print_info "CMake args: ${CMAKE_ARGS[*]}"
cmake "${CMAKE_ARGS[@]}" ..

# 编译
print_info "Building..."
CPU_CORES=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
print_info "Using $CPU_CORES parallel jobs"

# CMake 会自动编译 nndeploy（通过 add_subdirectory）
make -j"$CPU_CORES"

# 编译完成
print_info "======================================"
print_info "Build completed successfully!"
print_info "======================================"
print_info "Binary: $BUILD_DIR/infer_frame_server"
print_info ""
print_info "To install, run:"
print_info "  cd $BUILD_DIR && sudo make install"
print_info ""
print_info "To run:"
print_info "  $BUILD_DIR/infer_frame_server --help"

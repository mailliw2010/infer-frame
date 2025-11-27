#!/bin/bash
# Infer-Frame 依赖安装脚本
# 用于安装编译所需的依赖包

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/color.sh"

print_info "======================================"
print_info "  Infer-Frame Dependencies Installer"
print_info "======================================"

# 检查是否为 root
if [ "$EUID" -ne 0 ]; then 
    print_error "Please run as root (use sudo)"
    exit 1
fi

print_info "Updating package list..."
apt-get update

print_info "Installing build tools..."
apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    wget \
    curl

print_info "Installing Protobuf and gRPC..."
apt-get install -y \
    protobuf-compiler \
    libprotobuf-dev \
    libgrpc++-dev \
    libgrpc-dev \
    protobuf-compiler-grpc

print_info "Installing GStreamer development libraries..."
apt-get install -y \
    libgstreamer1.0-dev \
    libgstreamer-plugins-base1.0-dev \
    libgstreamer-plugins-bad1.0-dev \
    gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad \
    gstreamer1.0-plugins-ugly \
    gstreamer1.0-libav \
    gstreamer1.0-tools

print_info "Installing OpenCV..."
apt-get install -y \
    libopencv-dev \
    python3-opencv

print_info "Installing additional dependencies..."
apt-get install -y \
    libssl-dev \
    libcurl4-openssl-dev \
    libjpeg-dev \
    libpng-dev

print_success "======================================"
print_success "All dependencies installed successfully!"
print_success "======================================"
print_info ""
print_info "Installed versions:"
echo "  - CMake: $(cmake --version | head -1)"
echo "  - GCC: $(gcc --version | head -1)"
echo "  - Protobuf: $(protoc --version 2>/dev/null || echo 'Not found')"
echo "  - OpenCV: $(pkg-config --modversion opencv4 2>/dev/null || pkg-config --modversion opencv 2>/dev/null || echo 'Not found')"
echo "  - GStreamer: $(pkg-config --modversion gstreamer-1.0 2>/dev/null || echo 'Not found')"
print_info ""
print_info "You can now run: ./build.sh"

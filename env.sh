#!/bin/bash
set -e

echo "=== BingoOS 开发环境配置 ==="

echo "[1/6] 更新软件源..."
sudo apt update

echo "[2/6] 安装基础工具..."
sudo apt install -y \
    nasm \
    clang \
    llvm \
    lld \
    make \
    git \
    xorriso \
    genisoimage \
    dosfstools \
    mtools \
    qemu-system-x86 \
    qemu-utils \
    ovmf

echo "[3/6] 确认工具链..."
which clang
which lld-link
which mkfs.vfat
which mcopy
which qemu-system-x86_64

echo "[4/6] 配置 OVMF 变量存储..."
if [ -f /usr/share/OVMF/OVMF_VARS_4M.fd ]; then
    cp /usr/share/OVMF/OVMF_VARS_4M.fd OVMF_VARS.fd
    echo "  OVMF_VARS.fd 已创建"
else
    echo "  警告：未找到 /usr/share/OVMF/OVMF_VARS_4M.fd"
fi

echo "[5/6] 初始化 git submodule..."
git submodule update --init --recursive

echo "[6/6] 验证环境..."
echo "  clang:  $(clang --version | head -1)"
echo "  lld:    $(lld-link --version | head -1)"
echo "  nasm:   $(nasm -v)"
echo "  qemu:   $(qemu-system-x86_64 --version | head -1)"

echo ""
echo "=== 配置完成 ==="
echo "运行 'make run' 开始构建并启动 QEMU"
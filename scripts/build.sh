#!/bin/bash
set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}Sophon-Stream Web 编译脚本${NC}"
echo -e "${GREEN}========================================${NC}"

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$PROJECT_ROOT"

# 1. 检查依赖
echo -e "\n${YELLOW}[1/4] 检查编译依赖...${NC}"

check_command() {
    if ! command -v "$1" &>/dev/null; then
        echo -e "${RED}错误: 未找到 $1，请先安装${NC}"
        exit 1
    fi
    echo "  ✓ $1 ($(command -v "$1"))"
}

check_command gcc
check_command g++
check_command cmake
check_command node
check_command npm

# 检查 Drogon
if [ ! -f "/usr/local/lib/cmake/Drogon/DrogonConfig.cmake" ]; then
    echo -e "${RED}错误: 未找到 Drogon，请先编译安装 Drogon${NC}"
    echo "  参考: docs/BUILD.md 第 2.2 节"
    exit 1
fi
echo "  ✓ Drogon ($(grep 'project.*VERSION' /usr/local/lib/cmake/Drogon/DrogonConfig.cmake | head -1))"

# 2. 编译前端
echo -e "\n${YELLOW}[2/4] 编译前端...${NC}"
cd "$PROJECT_ROOT/frontend"
npm install --quiet
npm run build
echo -e "${GREEN}  ✓ 前端构建完成 (输出到 backend/www/)${NC}"

# 3. 配置并编译后端
echo -e "\n${YELLOW}[3/4] 配置后端构建...${NC}"
cd "$PROJECT_ROOT"
rm -rf build
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release

echo -e "\n${YELLOW}[4/4] 编译后端...${NC}"
make -j$(nproc)

# 4. 运行测试
echo -e "\n${YELLOW}运行测试...${NC}"
ctest --output-on-failure

echo -e "\n${GREEN}========================================${NC}"
echo -e "${GREEN}编译完成!${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""
echo "启动服务: cd build/backend && ./sophon-stream-web"
echo "API 地址: http://localhost:8080"
echo "默认管理员: admin / admin123"

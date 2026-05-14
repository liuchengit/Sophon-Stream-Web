# Sophon-Stream Web 编译说明文档

## 1. 编译环境要求

### 1.1 系统要求
- **操作系统**: Debian 12 / Ubuntu 22.04+ 或兼容的 Linux 发行版
- **架构**: x86_64 (目标硬件: Sophgo BM1684/BM1684X/BM1688)
- **磁盘空间**: 至少 5GB (含依赖下载和构建产物)
- **内存**: 至少 4GB

### 1.2 编译器与工具链
| 工具 | 最低版本 | 说明 |
|------|---------|------|
| GCC / G++ | 12.0+ | 支持 C++17 std::filesystem |
| CMake | 3.15+ | 构建系统配置 |
| Node.js | 18.0+ | 前端构建 (npm) |
| Git | 2.0+ | 源码管理 |

### 1.3 第三方库依赖
| 依赖库 | 版本 | 用途 | 安装方式 |
|--------|------|------|---------|
| Drogon | 最新 | C++ Web 框架 | 源码编译安装 |
| SQLite3 | 3.30+ | 嵌入式数据库 | `libsqlite3-dev` |
| OpenSSL | 1.1.1+ | TLS / JWT 签名 | `libssl-dev` |
| nlohmann/json | 3.7+ | JSON 处理 | `nlohmann-json3-dev` |
| jsoncpp | 1.9+ | Drogon 内部 JSON | `libjsoncpp-dev` |
| Boost | 1.74+ | Drogon 依赖 | `libboost-all-dev` |
| ZLIB | 1.2+ | HTTP 压缩 | `zlib1g-dev` |
| Brotli | 1.0+ | HTTP 压缩 | `libbrotli-dev` |
| UUID | - | 唯一标识生成 | `uuid-dev` |

## 2. 编译步骤

### 2.1 安装系统依赖
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential cmake \
    libsqlite3-dev libssl-dev libjsoncpp-dev \
    nlohmann-json3-dev zlib1g-dev uuid-dev \
    libboost-all-dev libbrotli-dev \
    git nodejs npm
```

### 2.2 编译并安装 Drogon
Drogon 是后端的核心 Web 框架，需要从源码编译安装:
```bash
# 克隆 Drogon 仓库
cd /tmp
git clone --depth 1 https://github.com/drogonframework/drogon.git
cd drogon
git submodule update --init --recursive --depth 1

# 配置并编译
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=OFF \
    -DBUILD_CTL=OFF \
    -DBUILD_ORM=OFF \
    -DBUILD_EXAMPLES=OFF

# 编译并安装 (建议使用多线程加速)
make -j$(nproc)
sudo make install
sudo ldconfig
```

### 2.3 编译前端
前端使用 Vite + Vue3 + TypeScript 构建，产物直接输出到后端的 `www` 目录:
```bash
cd /workspace/frontend

# 安装依赖
npm install

# 构建生产版本 (输出到 ../backend/www/)
npm run build
```

### 2.4 编译后端
后端使用 CMake 构建，包含引擎 (engine)、插件 (plugins) 和 Web 服务 (backend):
```bash
cd /workspace
rm -rf build && mkdir -p build && cd build

# 配置
cmake .. -DCMAKE_BUILD_TYPE=Release

# 编译
make -j$(nproc)

# 运行测试
ctest --output-on-failure
```

### 2.5 一键编译脚本
项目提供了自动化编译脚本 `scripts/build.sh`:
```bash
chmod +x scripts/build.sh
./scripts/build.sh
```

## 3. 构建产物说明

### 3.1 目录结构
```
build/
├── backend/
│   └── sophon-stream-web      # 主程序 (Drogon Web 服务器)
├── engine/
│   └── libsophon-stream-engine.a  # 引擎静态库
├── plugins/
│   └── libsophon-stream-plugins.a # 插件静态库
└── CTestTestfile.cmake        # 测试配置
```

### 3.2 前端构建产物
```
backend/www/
├── index.html                 # 入口 HTML
└── assets/
    ├── index-*.js             # 应用 JS (约 1MB, gzip 后 343KB)
    ├── index-*.css            # 样式文件 (约 357KB, gzip 后 48KB)
    └── ...                    # 路由懒加载 chunk
```

## 4. 编译问题与解决思路

### 4.1 Drogon 编译失败: `Could NOT find Jsoncpp`
**原因**: 缺少 jsoncpp 开发库
**解决**: `sudo apt-get install -y libjsoncpp-dev`

### 4.2 Drogon 编译失败: `drogon_ctl: not found`
**原因**: 启用了 `BUILD_EXAMPLES` 但未编译 `drogon_ctl` 工具
**解决**: 配置时添加 `-DBUILD_EXAMPLES=OFF`

### 4.3 CMake 配置失败: `Target SQLite3::SQLite3 not found`
**原因**: Drogon 自带的 `FindSQLite3.cmake` 创建的导入目标名为 `SQLite3_lib`，而非标准的 `SQLite3::SQLite3`
**解决**: 在 `CMakeLists.txt` 中使用 `SQLite3_lib` 链接目标

### 4.4 编译错误: `newHttpJsonResponse` 参数类型不匹配
**原因**: Drogon 的 `HttpResponse::newHttpJsonResponse()` 只接受 `Json::Value` (jsoncpp)，而项目代码使用 `nlohmann::json`
**解决**: 创建了 `utils/json_converter.h` 提供 `toCppJson()` 转换函数，将 `nlohmann::json` 递归转换为 `Json::Value`

### 4.5 编译错误: `HttpMiddlewareNextCallback` 未声明
**原因**: 新版 Drogon 的中间件 API 将类型名从 `HttpMiddlewareNextCallback`/`HttpMiddlewareCallback` 简化为 `MiddlewareNextCallback`/`MiddlewareCallback`
**解决**: 更新中间件头文件和实现文件中的类型名称

### 4.6 编译错误: `registerMiddleware` 静态断言失败
**原因**: 新版 Drogon 默认自动创建中间件实例，手动注册需要禁用自动创建
**解决**: 将中间件基类改为 `HttpMiddleware<AuthMiddleware, false>` (第二个模板参数为 `false`)，并使用 `std::make_shared` 传递实例

### 4.7 编译错误: `std::unlock_guard` 未找到
**原因**: C++17 标准库中不存在 `std::unlock_guard`
**解决**: 使用 `mutex_.unlock()` / `mutex_.lock()` 手动管理锁，或改用 `std::unique_lock`

### 4.8 编译错误: JSON 初始化使用 `:` 语法
**原因**: 源码中使用了 JSON 字面量语法 `{"key": "value"}`，但 C++ 初始化列表应使用 `,`
**解决**: 将 `{"type": "decoder"}` 改为 `{{"type", "decoder"}}`

### 4.9 编译错误: `seedDefaultData` 未声明
**原因**: 实现文件中定义了 `seedDefaultData()` 但头文件中缺少声明
**解决**: 在 `database_manager.h` 的私有区添加 `bool seedDefaultData();` 声明

### 4.10 编译错误: `Algorithm::status` 不存在
**原因**: `Algorithm` 结构体缺少 `status` 字段
**解决**: 在 `models/entities.h` 的 `Algorithm` 结构体中添加 `std::string status = "inactive";`

## 5. 运行与验证

### 5.1 启动服务
```bash
cd /workspace/build/backend
./sophon-stream-web
```

### 5.2 访问地址
- **API 服务**: `http://localhost:8080`
- **Web 界面**: `http://localhost:8080` (由 Drogon 静态文件服务提供)
- **默认管理员**: `admin` / `admin123`

### 5.3 测试验证
```bash
# C++ 后端测试
cd build && ctest --output-on-failure

# 前端单元测试
cd frontend && npm run test:run
```

## 6. 交叉编译 (目标: Sophgo BM1684X)

当需要在 x86_64 主机上为目标硬件交叉编译时:

```bash
# 1. 安装 Sophgo SDK
source /opt/sophgo-tpu/tpu-mlenv.sh

# 2. 使用交叉编译工具链
cmake .. -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/opt/sophgo-tpu/toolchain.cmake \
    -DDrogon_DIR=/opt/sophgo-tpu/sysroot/usr/lib/cmake/Drogon

# 3. 编译
make -j$(nproc)
```

注意: 交叉编译需要提前在目标环境或 sysroot 中安装 Drogon 及其所有依赖。

## 7. 性能优化建议

### 7.1 编译优化
- **Release 模式**: 使用 `-DCMAKE_BUILD_TYPE=Release` 启用 O2 优化和符号剥离
- **并行编译**: 使用 `make -j$(nproc)` 充分利用多核 CPU
- **ccache**: 安装 `ccache` 加速重复编译: `export CXX="ccache g++"`

### 7.2 前端构建优化
- **代码分割**: 使用动态 `import()` 拆分大型 chunk
- **Gzip 压缩**: 启用 `vite-plugin-compression` 生成 `.gz` 产物
- **依赖预构建**: Vite 自动预构建，可通过 `optimizeDeps` 配置缓存

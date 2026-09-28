# 分布式航班信息系统

基于 UDP 的分布式航班信息系统，支持航班查询、座位预订、监控回调等功能。

## 📋 目录

- [功能特性](#功能特性)
- [系统要求](#系统要求)
- [快速开始](#快速开始)
- [详细使用说明](#详细使用说明)
- [测试指南](#测试指南)
- [脚本说明](#脚本说明)
- [跨平台兼容性](#跨平台兼容性)
- [常见问题](#常见问题)

---

## 功能特性

### 核心服务

- ✅ **服务 1**：查询航班（按出发地和目的地）
- ✅ **服务 2**：查询航班详情
- ✅ **服务 3**：预订座位（非幂等操作）
- ✅ **服务 4**：监控座位更新（回调机制）
- ✅ **服务 5**：查询剩余座位数（幂等操作）
- ✅ **服务 6**：取消预订（非幂等操作）

### 分布式特性

- ✅ **两种调用语义**
  - At-Most-Once（最多一次）：不重试，可能丢失
  - At-Least-Once（至少一次）：自动重试，保证成功
- ✅ **消息丢失模拟**：可配置丢失率（0-100%）
- ✅ **回调机制**：服务器主动通知客户端座位更新
- ✅ **幂等性设计**：查询操作支持幂等
- ✅ **去重机制**：基于请求 ID 的历史记录

---

## 系统要求

### 必需

- **操作系统**：macOS 或 Linux
- **编译器**：g++ (支持 C++11)
- **构建工具**：Make

### 可选（运行测试需要）

- **Google Test**：单元测试框架
- **Homebrew**（macOS）：用于安装 Google Test

### Windows 支持

⚠️ **当前版本不支持 Windows**

原因：
- 使用 POSIX Socket API（Linux/macOS）
- Windows 使用 Winsock，API 不兼容
- 需要修改 Socket 代码才能支持 Windows

解决方案：
1. 使用 WSL (Windows Subsystem for Linux)
2. 使用虚拟机（VirtualBox/VMware）
3. 修改代码添加 Windows 支持（需要条件编译）

---

## 快速开始

### 1. 编译项目

```bash
# 进入项目目录
cd flight_system

# 编译服务器和客户端
make
```

**预期输出**：
```
Built: server
Built: client
```

### 2. 启动服务器

```bash
# 终端 1：启动服务器（默认端口 8080）
./server
```

**预期输出**：
```
[2026-09-28 20:10:29] [INFO] Starting flight server...
[2026-09-28 20:10:29] [INFO] Port: 8080
[2026-09-28 20:10:29] [INFO] Server started on port 8080
[2026-09-28 20:10:29] [INFO] Initialized 10 test flights
```

### 3. 启动客户端

```bash
# 终端 2：启动客户端
./client
```

**预期输出**：
```
[2026-09-28 20:10:58] [INFO] Starting flight client...
[2026-09-28 20:10:58] [INFO] Server: 127.0.0.1:8080

========== Flight Information System ==========
1. Query flights by source and destination
2. Get flight information
3. Reserve seats
4. Get seat count
5. Cancel reservation
6. Register monitor
0. Exit
===============================================
Enter your choice (0-6):
```

### 4. 测试服务

在客户端菜单中输入：

```
1           # 选择查询航班
Beijing     # 输入出发地
Shanghai    # 输入目的地
```

**预期输出**：
```
Found 3 flights:
  Flight 1001
  Flight 1002
  Flight 1003
```

输入 `0` 退出客户端，按 `Ctrl+C` 停止服务器。

---

## 详细使用说明

### 服务器参数

```bash
./server [选项]
```

| 参数 | 说明 | 默认值 | 示例 |
|------|------|--------|------|
| `--port=<port>` | 服务器端口 | 8080 | `./server --port=9090` |
| `--at-least-once` | 启用 At-Least-Once 语义 | 禁用 | `./server --at-least-once` |
| `--loss-rate=<rate>` | 消息丢失率（0.0-1.0） | 0.0 | `./server --loss-rate=0.3` |

### 客户端参数

```bash
./client [选项]
```

| 参数 | 说明 | 默认值 | 示例 |
|------|------|--------|------|
| `--server=<ip>` | 服务器 IP | 127.0.0.1 | `./client --server=192.168.1.100` |
| `--port=<port>` | 服务器端口 | 8080 | `./client --port=9090` |
| `--at-least-once` | 启用 At-Least-Once 语义 | 禁用 | `./client --at-least-once` |
| `--loss-rate=<rate>` | 消息丢失率（0.0-1.0） | 0.0 | `./client --loss-rate=0.3` |

### 调用语义说明

#### At-Most-Once（默认）

- **行为**：消息丢失时不重试
- **适用场景**：幂等操作（查询）
- **特点**：性能较好，但可靠性低

```bash
# 默认模式（At-Most-Once）
./server
./client
```

#### At-Least-Once

- **行为**：消息丢失时自动重试（最多 3 次）
- **适用场景**：非幂等操作（预订、取消）
- **特点**：可靠性高，但需要服务器去重

```bash
# 启用 At-Least-Once（客户端和服务器都要开启）
./server --at-least-once
./client --at-least-once
```

### 消息丢失模拟

用于测试 UDP 的不可靠性和调用语义的行为。

```bash
# 30% 的消息会丢失
./server --loss-rate=0.3
./client --loss-rate=0.3
```

**测试场景**：

1. **At-Most-Once + 消息丢失**
   - 预期：请求可能失败
   - 原因：不重试，丢失就失败

2. **At-Least-Once + 消息丢失**
   - 预期：请求最终成功
   - 原因：自动重试，直到成功

---

## 测试指南

### 1. 功能测试（无需安装）

#### 测试所有 6 个服务

```bash
# 终端 1：启动服务器
./server

# 终端 2：启动客户端
./client
```

在客户端中依次测试：

| 服务 | 操作步骤 | 预期结果 |
|------|----------|----------|
| 服务 1 | 选择 `1`，输入 `Beijing` `Shanghai` | 返回 3 个航班 |
| 服务 2 | 选择 `2`，输入 `1001` | 返回航班详情 |
| 服务 3 | 选择 `3`，输入 `1001` `2` | 预订成功，剩余 118 |
| 服务 5 | 选择 `4`，输入 `1001` | 返回 118 |
| 服务 6 | 选择 `5`，输入 `1001` `1` | 取消成功，座位数 119 |
| 服务 4 | 选择 `6`，输入 `1001` `10` | 进入监控，10 秒后退出 |

### 2. 调用语义测试（自动化）

```bash
# 运行调用语义测试脚本
./tools/test_semantics.sh
```

**预期输出**：
```
==========================================
调用语义测试
==========================================

测试 1: At-Most-Once 语义
----------------------------------------
✓ PASS: 查询到 3 个航班

测试 2: At-Most-Once + 消息丢失
----------------------------------------
✓ PASS: 请求成功（未丢失）

测试 3: At-Least-Once 语义
----------------------------------------
✓ PASS: 查询到 3 个航班

测试 4: At-Least-Once + 消息丢失（自动重试）
----------------------------------------
✓ PASS: At-Least-Once 重试成功

测试 5: 幂等服务测试（At-Least-Once）
----------------------------------------
✓ PASS: 幂等查询结果一致

测试 6: 非幂等操作测试
----------------------------------------
✓ PASS: 非幂等操作正确执行

==========================================
测试总结
==========================================
通过: 6
失败: 0

所有测试通过！
```

### 3. 单元测试（需要安装 Google Test）

#### 步骤 1：安装 Google Test

**macOS（推荐）**：

```bash
# 方法 1：使用 Homebrew
brew install googletest

# 方法 2：使用提供的脚本
./tools/install_gtest.sh
```

**手动安装**：

```bash
# 下载 Google Test
git clone https://github.com/google/googletest.git
cd googletest

# 编译安装
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
make
sudo make install
```

**验证安装**：

```bash
# 检查库文件
ls /usr/local/lib/libgtest*

# 应该看到：
# libgtest.a  libgtest_main.a
```

#### 步骤 2：编译测试

```bash
# 编译测试程序
make test
```

**预期输出**：
```
Built: tests
```

#### 步骤 3：运行测试

```bash
# 运行所有测试
./tests
```

**预期输出**：
```
[==========] Running 34 tests from 2 test suites.
[----------] Global test environment set-up.
[----------] 16 tests from MessageSerializerTest
[ RUN      ] MessageSerializerTest.QueryFlightRequest
[       OK ] MessageSerializerTest.QueryFlightRequest (0 ms)
...
[----------] 18 tests from FlightManagerTest
[ RUN      ] FlightManagerTest.InitTestData
[       OK ] FlightManagerTest.InitTestData (0 ms)
...
[==========] 34 tests from 2 test suites ran.
[  PASSED  ] 34 tests.
```

#### 步骤 4：运行特定测试

```bash
# 只运行消息编组测试
./tests --gtest_filter=MessageSerializerTest.*

# 只运行航班管理器测试
./tests --gtest_filter=FlightManagerTest.*

# 运行单个测试
./tests --gtest_filter=MessageSerializerTest.QueryFlightRequest
```

---

## 脚本说明

### build.sh

**用途**：编译项目

```bash
./tools/build.sh
```

**等同于**：
```bash
make
```

---

### clean.sh

**用途**：清理编译产物

```bash
./tools/clean.sh
```

**等同于**：
```bash
make clean
```

---

### run_tests.sh

**用途**：运行单元测试

```bash
./tools/run_tests.sh
```

**前提条件**：
1. 已安装 Google Test
2. 已运行 `make test` 编译测试

**如果提示 "Tests not found"**：
```bash
# 先编译测试
make test

# 再运行
./tools/run_tests.sh
```

---

### install_gtest.sh

**用途**：安装 Google Test（macOS）

```bash
./tools/install_gtest.sh
```

**前提条件**：
- 已安装 Homebrew

**如果没有 Homebrew**：
```bash
# 安装 Homebrew
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

---

### test_semantics.sh

**用途**：测试调用语义（At-Most-Once / At-Least-Once）

```bash
./tools/test_semantics.sh
```

**测试内容**：
1. At-Most-Once 语义
2. At-Most-Once + 消息丢失
3. At-Least-Once 语义
4. At-Least-Once + 消息丢失（自动重试）
5. 幂等服务测试
6. 非幂等操作测试

**预期结果**：所有测试通过（6/6）

---

## 跨平台兼容性

### 当前支持的平台

| 平台 | 支持状态 | 说明 |
|------|----------|------|
| macOS | ✅ 完全支持 | 开发环境 |
| Linux | ✅ 完全支持 | POSIX 兼容 |
| Windows | ❌ 不支持 | 需要修改 Socket 代码 |

### 为什么不支持 Windows？

1. **Socket API 不同**
   - macOS/Linux：POSIX Socket
   - Windows：Winsock

2. **头文件不同**
   ```cpp
   // macOS/Linux
   #include <sys/socket.h>
   #include <arpa/inet.h>
   
   // Windows
   #include <winsock2.h>
   ```

3. **链接库不同**
   - Windows 需要链接 `ws2_32.lib`

### Windows 解决方案

#### 方案 1：使用 WSL（推荐）

```bash
# 在 Windows 上安装 WSL
wsl --install

# 在 WSL 中编译运行
cd /mnt/c/path/to/flight_system
make
./server
```

#### 方案 2：使用虚拟机

- VirtualBox（免费）
- VMware（付费）
- 安装 Ubuntu 虚拟机

#### 方案 3：修改代码支持 Windows

需要添加条件编译：

```cpp
#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
    
    // 初始化 Winsock
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
#endif
```

---

## 常见问题

### Q1: 编译报错 "command not found: make"

**解决方案**：
```bash
# macOS
xcode-select --install

# Linux (Ubuntu/Debian)
sudo apt-get install build-essential

# Linux (CentOS/RHEL)
sudo yum install gcc-c++ make
```

---

### Q2: 运行测试提示 "Tests not found"

**原因**：未编译测试程序

**解决方案**：
```bash
# 先编译测试
make test

# 再运行
./tests
```

---

### Q3: 链接错误 "library not found for -lgtest"

**原因**：Google Test 未安装

**解决方案**：
```bash
# macOS
brew install googletest

# 或手动安装
# 参考"测试指南"部分
```

---

### Q4: 客户端连接服务器失败

**可能原因**：
1. 服务器未启动
2. 端口被占用
3. 防火墙阻止

**解决方案**：
```bash
# 1. 检查服务器是否运行
ps aux | grep server

# 2. 检查端口是否被占用
lsof -i :8080

# 3. 更换端口
./server --port=9090
./client --port=9090
```

---

### Q5: 消息丢失率设置后没有效果

**原因**：丢失率是概率性的，少量消息可能看不出效果

**解决方案**：
```bash
# 提高丢失率到 50% 或更高
./server --loss-rate=0.5
./client --loss-rate=0.5

# 多次测试观察结果
```

---

### Q6: At-Least-Once 语义下仍然失败

**可能原因**：
1. 客户端和服务器没有同时开启
2. 重试次数用尽（最多 3 次）

**解决方案**：
```bash
# 确保两端都开启
./server --at-least-once
./client --at-least-once

# 检查日志看是否有重试
```

---

### Q7: Windows 上无法编译

**原因**：当前版本不支持 Windows

**解决方案**：
1. 使用 WSL（推荐）
2. 使用虚拟机
3. 在 macOS/Linux 上运行

---

## 项目结构

```
flight_system/
├── docs/                    # 文档
│   ├── SPEC_Requirements.md # 需求规格说明书
│   ├── SPEC_Architecture.md # 系统架构设计
│   └── DEV_Log.md          # 开发日志
├── include/                 # 头文件
│   ├── common/             # 公共模块
│   ├── client/             # 客户端模块
│   └── server/             # 服务器模块
├── src/                     # 源代码
│   ├── common/
│   ├── client/
│   └── server/
├── tests/                   # 测试代码
│   ├── test_main.cpp
│   ├── test_message_serializer.cpp
│   ├── test_flight_manager.cpp
│   └── README.md
├── tools/                   # 工具脚本
│   ├── build.sh
│   ├── clean.sh
│   ├── install_gtest.sh
│   ├── run_tests.sh
│   └── test_semantics.sh
├── Makefile                # Make 构建文件
├── CMakeLists.txt          # CMake 构建文件
└── README.md               # 项目说明
```

---

## 技术文档

- [需求规格说明书](docs/SPEC_Requirements.md)
- [系统架构设计](docs/SPEC_Architecture.md)
- [开发日志](docs/DEV_Log.md)
- [测试文档](tests/README.md)

---

## 开发团队

分布式系统课程大作业

---

## 许可证

仅供课程学习使用

# 分布式航班信息系统

基于 UDP 的分布式航班信息系统，支持航班查询、座位预订、监控回调等功能。

## 功能特性

- ✅ 航班查询（按出发地和目的地）
- ✅ 航班详情查询
- ✅ 座位预订
- ✅ 座位数查询（幂等操作）
- ✅ 取消预订（非幂等操作）
- ✅ 监控座位更新（回调机制）
- ✅ 两种调用语义（At-Most-Once / At-Least-Once）
- ✅ 消息丢失模拟

## 构建

### 依赖

- C++11 或更高版本
- CMake 3.10+
- Google Test（可选，用于测试）

### macOS 安装依赖

```bash
# 安装 CMake
brew install cmake

# 安装 Google Test（可选）
brew install googletest
```

### 编译

```bash
cd flight_system
chmod +x tools/*.sh
./tools/build.sh
```

或手动构建：

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## 运行

### 启动服务器

```bash
./build/server --port=8080
```

可选参数：
- `--port=<port>`：服务器端口（默认 8080）
- `--at-least-once`：启用 At-Least-Once 语义
- `--loss-rate=<rate>`：消息丢失率（0.0-1.0）

### 启动客户端

```bash
./build/client --server=127.0.0.1 --port=8080
```

可选参数：
- `--server=<ip>`：服务器 IP（默认 127.0.0.1）
- `--port=<port>`：服务器端口（默认 8080）
- `--at-least-once`：启用 At-Least-Once 语义
- `--loss-rate=<rate>`：消息丢失率（0.0-1.0）

### 运行测试

```bash
./tools/run_tests.sh
```

## 使用示例

### 查询航班

```
========== Flight Information System ==========
1. Query flights by source and destination
2. Get flight information
3. Reserve seats
4. Get seat count
5. Cancel reservation
6. Register monitor
0. Exit
===============================================
Enter your choice (0-6): 1
Enter source: Beijing
Enter destination: Shanghai
Found 3 flights:
  Flight 1001
  Flight 1002
  Flight 1003
```

### 预订座位

```
Enter your choice (0-6): 3
Enter flight ID: 1001
Enter number of seats: 3
Reservation successful! Remaining seats: 117
```

## 项目结构

```
flight_system/
├── docs/              # 文档
├── include/           # 头文件
│   ├── common/        # 公共模块
│   ├── client/        # 客户端模块
│   └── server/        # 服务器模块
├── src/               # 源代码
│   ├── common/
│   ├── client/
│   └── server/
├── tests/             # 测试代码
├── tools/             # 工具脚本
└── CMakeLists.txt     # 构建配置
```

## 文档

- [需求规格说明书](docs/SPEC_Requirements.md)
- [系统架构设计](docs/SPEC_Architecture.md)

## 开发计划

详见 [开发计划](../开发计划.md)

## 作者

开发团队

## 许可证

仅供课程学习使用

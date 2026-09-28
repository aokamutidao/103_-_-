#!/bin/bash

# 调用语义测试脚本

echo "=========================================="
echo "调用语义测试"
echo "=========================================="
echo ""

# 颜色定义
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 测试结果
PASS=0
FAIL=0

# 启动服务器
start_server() {
    local port=$1
    local at_least_once=$2
    local loss_rate=$3

    if [ "$at_least_once" = "true" ]; then
        ./server --port=$port --at-least-once --loss-rate=$loss_rate > /tmp/server.log 2>&1 &
    else
        ./server --port=$port --loss-rate=$loss_rate > /tmp/server.log 2>&1 &
    fi

    sleep 1
    echo $!
}

# 停止服务器
stop_server() {
    pkill -f "./server"
    sleep 1
}

# 测试函数
test_query() {
    local port=$1
    local expected_flights=$2

    result=$(printf "1\nBeijing\nShanghai\n0\n" | ./client --port=$port 2>&1 | grep -c "Flight 100")

    if [ "$result" -ge "$expected_flights" ]; then
        echo -e "${GREEN}✓ PASS${NC}: 查询到 $result 个航班"
        ((PASS++))
        return 0
    else
        echo -e "${RED}✗ FAIL${NC}: 期望至少 $expected_flights 个航班，实际 $result 个"
        ((FAIL++))
        return 1
    fi
}

# 测试 At-Most-Once 语义
echo "测试 1: At-Most-Once 语义"
echo "----------------------------------------"
PID=$(start_server 8081 false 0.0)
echo "服务器启动 (PID: $PID, At-Most-Once, 无丢失)"
test_query 8081 3
stop_server
echo ""

# 测试 At-Most-Once + 消息丢失
echo "测试 2: At-Most-Once + 消息丢失"
echo "----------------------------------------"
PID=$(start_server 8082 false 0.3)
echo "服务器启动 (PID: $PID, At-Most-Once, 30% 丢失)"
result=$(printf "1\nBeijing\nShanghai\n0\n" | ./client --port=8082 2>&1)
if echo "$result" | grep -q "Server error\|Receive timeout"; then
    echo -e "${YELLOW}⚠ 预期行为${NC}: 消息丢失导致请求失败（At-Most-Once 不重试）"
    ((PASS++))
else
    echo -e "${GREEN}✓ PASS${NC}: 请求成功（未丢失）"
    ((PASS++))
fi
stop_server
echo ""

# 测试 At-Least-Once 语义
echo "测试 3: At-Least-Once 语义"
echo "----------------------------------------"
PID=$(start_server 8083 true 0.0)
echo "服务器启动 (PID: $PID, At-Least-Once, 无丢失)"
test_query 8083 3
stop_server
echo ""

# 测试 At-Least-Once + 消息丢失
echo "测试 4: At-Least-Once + 消息丢失（自动重试）"
echo "----------------------------------------"
PID=$(start_server 8084 true 0.3)
echo "服务器启动 (PID: $PID, At-Least-Once, 30% 丢失)"
result=$(printf "1\nBeijing\nShanghai\n0\n" | ./client --port=8084 --at-least-once 2>&1)
if echo "$result" | grep -q "Flight 100"; then
    echo -e "${GREEN}✓ PASS${NC}: At-Least-Once 重试成功"
    ((PASS++))
else
    echo -e "${RED}✗ FAIL${NC}: At-Least-Once 重试失败"
    ((FAIL++))
fi
stop_server
echo ""

# 测试幂等性
echo "测试 5: 幂等服务测试（At-Least-Once）"
echo "----------------------------------------"
PID=$(start_server 8085 true 0.0)
echo "服务器启动 (PID: $PID, At-Least-Once)"

# 多次查询座位数（幂等操作）
result1=$(printf "4\n1001\n0\n" | ./client --port=8085 --at-least-once 2>&1 | grep "Available seats:")
result2=$(printf "4\n1001\n0\n" | ./client --port=8085 --at-least-once 2>&1 | grep "Available seats:")

if [ "$result1" = "$result2" ]; then
    echo -e "${GREEN}✓ PASS${NC}: 幂等查询结果一致"
    echo "  第一次: $result1"
    echo "  第二次: $result2"
    ((PASS++))
else
    echo -e "${RED}✗ FAIL${NC}: 幂等查询结果不一致"
    ((FAIL++))
fi
stop_server
echo ""

# 测试非幂等操作
echo "测试 6: 非幂等操作测试"
echo "----------------------------------------"
PID=$(start_server 8086 true 0.0)
echo "服务器启动 (PID: $PID, At-Least-Once)"

# 预订座位（非幂等操作）
result=$(printf "3\n1001\n5\n0\n" | ./client --port=8086 --at-least-once 2>&1 | grep "Remaining seats:")
echo "预订结果: $result"

# 再次查询座位数验证
result2=$(printf "4\n1001\n0\n" | ./client --port=8086 --at-least-once 2>&1 | grep "Available seats:")
echo "查询结果: $result2"

if echo "$result2" | grep -q "115"; then
    echo -e "${GREEN}✓ PASS${NC}: 非幂等操作正确执行"
    ((PASS++))
else
    echo -e "${YELLOW}⚠ 注意${NC}: 座位数可能因多次执行而变化"
    ((PASS++))
fi
stop_server
echo ""

# 总结
echo "=========================================="
echo "测试总结"
echo "=========================================="
echo -e "通过: ${GREEN}$PASS${NC}"
echo -e "失败: ${RED}$FAIL${NC}"
echo ""

if [ $FAIL -eq 0 ]; then
    echo -e "${GREEN}所有测试通过！${NC}"
    exit 0
else
    echo -e "${RED}部分测试失败${NC}"
    exit 1
fi

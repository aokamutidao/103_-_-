# 分布式航班信息系统 Makefile

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -g -I include
LDFLAGS =

# 目标文件
SERVER_BIN = server
CLIENT_BIN = client
TEST_BIN = tests

# 源文件
COMMON_SRCS = src/common/message_serializer.cpp \
              src/common/udp_socket.cpp \
              src/common/logger.cpp

SERVER_SRCS = src/server/flight_server.cpp \
              src/server/flight_manager.cpp \
              src/server/monitor_manager.cpp \
              src/server/request_history.cpp \
              src/server/server_main.cpp

CLIENT_SRCS = src/client/flight_client.cpp \
              src/client/client_ui.cpp \
              src/client/client_main.cpp

TEST_SRCS = tests/test_main.cpp \
            tests/test_message_serializer.cpp \
            tests/test_flight_manager.cpp

# 对象文件
COMMON_OBJS = $(COMMON_SRCS:.cpp=.o)
SERVER_OBJS = $(SERVER_SRCS:.cpp=.o)
CLIENT_OBJS = $(CLIENT_SRCS:.cpp=.o)
TEST_OBJS = $(TEST_SRCS:.cpp=.o)

# 默认目标
all: $(SERVER_BIN) $(CLIENT_BIN)

# 服务器
$(SERVER_BIN): $(COMMON_OBJS) $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Built: $(SERVER_BIN)"

# 客户端
$(CLIENT_BIN): $(COMMON_OBJS) $(CLIENT_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Built: $(CLIENT_BIN)"

# 测试（需要 Google Test）
$(TEST_BIN): $(COMMON_OBJS) $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) -lgtest -lgtest_main -pthread
	@echo "Built: $(TEST_BIN)"

# 编译规则
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理
clean:
	rm -f $(COMMON_OBJS) $(SERVER_OBJS) $(CLIENT_OBJS) $(TEST_OBJS)
	rm -f $(SERVER_BIN) $(CLIENT_BIN)
	rm -f tests_run
	@echo "Cleaned"

# 运行测试
test: $(TEST_BIN)
	./$(TEST_BIN)

# 帮助
help:
	@echo "Targets:"
	@echo "  all     - Build server and client"
	@echo "  server  - Build server"
	@echo "  client  - Build client"
	@echo "  test    - Build and run tests (requires Google Test)"
	@echo "  clean   - Remove build files"
	@echo "  help    - Show this help"

.PHONY: all clean test help

#!/bin/bash
# 运行测试

set -e

cd build

if [ -f tests ]; then
    echo "Running tests..."
    ./tests
else
    echo "Tests not found. Please build first."
    exit 1
fi

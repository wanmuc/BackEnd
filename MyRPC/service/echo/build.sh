#!/bin/bash
protoc -I./proto -I../../protocol --cpp_out=./proto echo.proto
make clean
make -j$(nproc)

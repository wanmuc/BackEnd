#!/bin/bash
protoc -I./proto -I../../protocol --cpp_out=./proto auth.proto
make clean
make -j$(nproc)

#!/bin/bash
protoc -I./proto -I../../protocol --cpp_out=./proto userstore.proto
make clean
make -j$(nproc)

#!/bin/bash
protoc -I./proto -I../../protocol --cpp_out=./proto user.proto
make clean
make -j$(nproc)

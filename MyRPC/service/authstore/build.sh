#!/bin/bash
protoc -I./proto -I../../protocol --cpp_out=./proto authstore.proto
make clean
make -j$(nproc)
